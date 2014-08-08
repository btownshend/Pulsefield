#include "lasers.h"
#include "connections.h"
#include "person.h"
#include "groups.h"
#include "video.h"

static const float LASERSEP=0.5f;
static const float TARGETSEP=0.6f;

std::shared_ptr<Lasers> Lasers::theInstance;   // Singleton

Lasers::Lasers(int nlasers): lasers(nlasers) {
    etherdream_lib_start();

    dbg("Lasers.Lasers",1) << "Constructing " << nlasers << " lasers." << std::endl;
    for (unsigned int i=0;i<lasers.size();i++) {
	lasers[i]=std::shared_ptr<Laser>(new Laser(i));
	lasers[i]->open();
    }
    needsRender=true;
    if (pthread_mutex_init(&mutex,NULL)) {
	std::cerr<<"Failed to create lasers mutex" << std:: endl;
	exit(1);
    }
    
    setFlag("background",false);
    setFlag("grid",false);
    setFlag("alignment",false);
    setFlag("body",true);
    setFlag("legs",true);
}

Lasers::~Lasers() {
    (void)pthread_mutex_destroy(&mutex);
}

// Allocate a drawing to the lasers
std::vector<Drawing> Lasers::allocate(Drawing &d, const Ranges &ranges)  const {
    static const float MINSHADOWTODROP=0.5;
    static const float FRACFUZZ=0.1;	// Fuzz in fractions -- fractions within this amount are considered equal
    static const float DISTFUZZ=3.0;   // Fuzz in distance of laser -- distances within this amount are considered equal

    std::vector<Drawing> result(lasers.size());
    for (int el=0;el<d.getNumElements();el++) {
	std::shared_ptr<Composite> &c=d.getElement(el);

	// Get stats for drawing this composite from each of the lasers
	// stats[i].fracOnScreen will be -1 for disabled lasers
	std::vector<LaserStat> stats=computeStats(*c,ranges);
	int current=c->getShapeID()->getLaser();	// Index of stats for currently assigned laser
	int assignment=-1;

	do { // Dummy loop so we can do a break
	    // If it is mostly obstructed, remove the assignment (since we are now free to assign at will)
	    if (current >=0 && stats[current].fracShadowed*(1-stats[current].fracOnScreen) >= MINSHADOWTODROP) {
		dbg("Lasers.allocate",1) << "Ignoring assignment to " << current << " since visible frac  = " << stats[current].fracShadowed*(1-stats[current].fracOnScreen) << ", which is >= " << MINSHADOWTODROP << std::endl;
		current=-1;
	    }
	    float maxFrac=stats[0].fracOnScreen*(1-stats[0].fracShadowed);
	    for (int j=1;j<stats.size();j++)
		maxFrac=std::max(maxFrac,stats[j].fracOnScreen*(1-stats[j].fracShadowed));
	    if (maxFrac==0)
		// No assignment that works
		break;
	    if (current>=0 && stats[current].fracOnScreen*(1-stats[current].fracShadowed) >= maxFrac-FRACFUZZ)  {
		dbg("Lasers.allocate",2) << "Keeping assignment to laser " << current << " since frac visible=" << stats[current].fracOnScreen*(1-stats[current].fracShadowed) << " is, or is close to, the best which is " << maxFrac << std::endl;
		assignment=current;
		break;
	    }
	    // Assign to least loaded  laser within FRACFUZZ of maxFrac, within DISTFUZZ of closest laser, 
	    float bestDist=1e10;
	    float  bestLength=1000;
	    for (int j=0;j<stats.size();j++) {
		if (stats[j].fracOnScreen*(1-stats[j].fracShadowed) >= maxFrac-FRACFUZZ && stats[j].meanDistance < bestDist+DISTFUZZ) {
		    if (stats[j].meanDistance < bestDist-DISTFUZZ || result[j].getLength()<bestLength) {
			assignment=j;
			bestDist=stats[j].meanDistance;
			bestLength=result[j].getLength();
		    }
		}
	    }
	} while (false);
	if (assignment==-1) {
	    dbg("Lasers.allocate",1) << "No assignment possible for " << c->getShapeID()->getID() << ":skipping" << std::endl;
	} else {
	    // Make assignment
	    float len=result[assignment].getLength();
	    dbg("Lasers.allocate",1) << "Assigned " << c->getShapeID()->getID() << " to laser " << assignment << "(frac=" << stats[assignment].fracOnScreen << ", shadow=" << stats[assignment].fracShadowed << ", dist=" << stats[assignment].meanDistance << ", total length for laser so far=" << len << ")" << std::endl;
	    // Remove shadowed points
	    c->setPoints(lasers[assignment]->removeShadowed(c->getPoints(),ranges,LASERSEP,TARGETSEP));
	    // Make the assignment
	    result[assignment].append(c );
	    // Mark it in the shapeID
	    c->getShapeID()->setLaser(assignment);
	}
    }
    return result;
}

int Lasers::render(const Ranges &ranges, const Bounds  &bounds) {
    if (!needsRender) {
	dbg("Lasers.render",5) << "Not dirty" << std::endl;
	return 0;
    }
    lock();
    needsRender=false;

    Drawing drawing;
    if (visual!=nullptr)
	drawing.append(*visual);	// Background
    Connections::draw(drawing);
    Groups::draw(drawing);
    People::draw(drawing);
    dbg("Lasers.render",1) << "BG+People+Connections+Groups have " << drawing.getNumElements() << " elements." << std::endl;

    if (getFlag("allocationTest")) {
	// Allocation test pattern
	// Draw circles over a uniform grid to see where they get allocated
	static const int ngrid=13;
	// Assumes active region is 7m x 14m
	static const float width=13.5;
	static const float depth=6.5;
	static const float radius=0.1;

	drawing.shapeBegin("allocationTest",Attributes());
	float minx=-width/2+0.5f,maxx=width/2-0.5f,miny=0.5f,maxy=depth-0.5f;
	float xstep=(maxx-minx)/(ngrid-1);
	float ystep=(maxy-miny)/(ngrid-1);
	for (int i=0;i<ngrid;i++) {
	    float x=minx+i*xstep;
	    for (int j=0;j<ngrid;j++) {
		float y=miny+j*ystep;
		drawing.drawCircle(Point(x,y),radius,Color(0.0,1.0,0.0));
	    }
	}
	drawing.shapeEnd("allocationTest");
	dbg("Lasers.render",1) << "After adding allocation test pattern, have " << drawing.getNumElements() << " elements." << std::endl;
    }

    // Split drawing among lasers
    static const float RASTERIZESPACING=0.01;	// Initial rasterization spacing
    drawing.rasterize(RASTERIZESPACING);    // Rasterize
    drawing.clip(bounds);	// Clip to active area
    std::vector<Drawing> dtmp=allocate(drawing,ranges);

    // Global drawing - applies to all lasers 
    Drawing globalDrawing;

    const Color bgColor=Color(0.0,1.0,0.0);
    const Color gridColor=Color(0.0,1.0,0.0);

    if (getFlag("background") && ranges.size()>0) {
	globalDrawing.shapeBegin("backgroundTest",Attributes());
	Point prevp(0,0);
	for (int i=0;i<ranges.size();i++) {
	    Point p=ranges.getPoint(i);
	    if (bounds.contains(p)) {
		Point p1,p2;
		// Make sure to order lines so the ends will be close together in order draw to minimize laser slewing
		p1.setThetaRange(p.getTheta()-ranges.getScanRes()/2,p.norm());
		p2.setThetaRange(p.getTheta()+ranges.getScanRes()/2,p.norm());
		globalDrawing.drawLine(p1,p2,bgColor);
	    }
	}
	globalDrawing.shapeEnd("backgroundTest");
    }
    if (getFlag("alignment") && background.size()>0)  {
	// TODO: Draw alignment pattern
	static const float MINTARGETDISTFROMBG=0.5;   // Minimum distance of target from background
	static const float MAXTARGETRANGEDIFF=0.3;
	static const int MINTARGETHITS=2;	// Minimum number of hits for it to be a target
	static const int MAXTARGETHITS=10;	// Maximum number of hits for it to be a target
	float lastBgRange=background[0].norm();
	int inTargetCnt=0;
	float tgtRange=0;
	globalDrawing.shapeBegin("alignmentTest",Attributes());
	for (int i=0;i<background.size();i++) {
	    float range=background[i].norm();
	    dbg("Laser.showAlignment",10) <<  "i=" << i << ", range=" << range << ", inTargetCnt=" << inTargetCnt << std::endl;
	    if (inTargetCnt>0 && fabs(range-tgtRange)>MAXTARGETRANGEDIFF)  {
	      if (inTargetCnt>=MINTARGETHITS) {
		// Just finished a target
		float diameter=(background[i-1]-background[i-inTargetCnt]).norm()*(inTargetCnt+1)/inTargetCnt;
		Point center=(background[i-inTargetCnt/2-1]+background[i-(inTargetCnt+1)/2])/2;
		center=center * (tgtRange+diameter/2)/center.norm();	// Move to correct range
		dbg("Laser.showAlignment",3) << "alignment pattern detected at scans " << i-inTargetCnt << "-" << i-1 << " at " << center << " with diameter " << diameter << " at range " << tgtRange <<  " with background at range " << lastBgRange << ", new range=" << range <<  std::endl;
		// Draw the hits on the target as a polygon
		std::vector<Point> tgt(background.begin()+i-inTargetCnt,background.begin()+i);
		globalDrawing.drawPolygon(tgt,bgColor);
		// Draw a circle around target at a larger radius
		globalDrawing.drawCircle(center,diameter*0.6,bgColor);
	      } 
	      inTargetCnt=0;  // Reset, since this is a different range
	    }
	    if (range<lastBgRange-MINTARGETDISTFROMBG) {
	      if (inTargetCnt==0)
		tgtRange=range;
	      else
		tgtRange=std::min(range,tgtRange);
	      inTargetCnt++;
	    } else {
		lastBgRange=range;
		inTargetCnt=0;
	    }
	    if (inTargetCnt> MAXTARGETHITS) {
	      // Too big -- reset
	      inTargetCnt=0;
	      lastBgRange=range;
	    }
	}
	globalDrawing.shapeEnd("alignmentTest");
    }
    if (getFlag("grid")) {
	int ngrid=7;
	// Assumes active region is 7m x 14m
	float width=13;
	float depth=6;
	float minx=-width/2+0.5f,maxx=width/2-0.5f,miny=0.5f,maxy=depth-0.5f;
	float xstep=(maxx-minx)/(ngrid-1);
	globalDrawing.shapeBegin("gridTest",Attributes());
	for (int i=0;i<ngrid;i++) {
	    float x=minx+i*xstep;
	    globalDrawing.drawLine(Point(x,miny),Point(x,maxy),gridColor);
	    if (x+xstep<=maxx)  {
		globalDrawing.drawLine(Point(x,maxy),Point(x+xstep,maxy),gridColor);
		globalDrawing.drawLine(Point(x+xstep,maxy),Point(x+xstep,miny),gridColor);
	    }
	    if (x+2*xstep<=maxx)
		globalDrawing.drawLine(Point(x+xstep,miny),Point(x+2*xstep,miny),gridColor);
	}
	float ystep=(maxy-miny)/(ngrid-1);
	for (int j=0;j<ngrid;j++) {
	    float y=miny+j*ystep;
	    globalDrawing.drawLine(Point(minx,y),Point(maxx,y),gridColor);
	    if (y+ystep<=maxy) {
		globalDrawing.drawLine(Point(maxx,y),Point(maxx,y+ystep),gridColor);
		globalDrawing.drawLine(Point(maxx,y+ystep),Point(minx,y+ystep),gridColor);
	    }
	    if (y+2*ystep<=maxy)
		globalDrawing.drawLine(Point(minx,y+ystep),Point(minx,y+2*ystep),gridColor);
	}
	globalDrawing.shapeEnd("gridTest");
    }
    globalDrawing.rasterize(RASTERIZESPACING);
    globalDrawing.clip(bounds);
    for (unsigned int i=0;i<lasers.size();i++) {
	if (getFlag("outline") && lasers[i]->isEnabled()) {
	    lasers[i]->showOutline(bounds);
	}
	else if (getFlag("test") && lasers[i]->isEnabled()) {
	    lasers[i]->showTest();
	} else {
	    dtmp[i].append(globalDrawing);
	    lasers[i]->render(dtmp[i]);
	}
    }
    dbg("Lasers.render",1) << "Render done" << std::endl;
    unlock();
    return 1;
}

void Lasers::saveTransforms(std::ostream &s)  const {
    for (unsigned int i=0;i<lasers.size();i++)
	lasers[i]->getTransform().save(s);
}

void Lasers::loadTransforms(std::istream &s) {
    for (unsigned int i=0;i<lasers.size();i++)
	lasers[i]->getTransform().load(s);
    needsRender=true;
}

void Lasers::clearTransforms(const Bounds &floorBounds) {
    for (unsigned int i=0;i<lasers.size();i++)
	lasers[i]->getTransform().clear(floorBounds);
    needsRender=true;
}

void Lasers::lock() {
    dbg("Lasers.lock",5) << "lock req" << std::endl;
    if (pthread_mutex_lock(&mutex)) {
	std::cerr << "Failed call to pthread_mutex_lock" << std::endl;
	exit(1);
    }
    dbg("Lasers.lock",5) << "lock acquired" << std::endl;
}

void Lasers::unlock() {
    dbg("Lasers.unlock",5) << "unlock" << std::endl;
    if (pthread_mutex_unlock(&mutex)) {
	std::cerr << "Failed call to pthread_mutex_lock" << std::endl;
	exit(1);
    }
}

void Lasers::setBackground(int scanpt, int totalpts, float angleDeg, float range) {
    dbg("Lasers.setBackground",5) << "scanpt=" << scanpt << "/" << totalpts << ", angle=" <<angleDeg <<", range=" << range << std::endl;
    if (totalpts != background.size()) {
	dbg("Lasers.setBackground",1) << "resize background to " << totalpts << " points" << std::endl;
	background.resize(totalpts);
    }
    if (range>0.1)
	background[scanpt].setThetaRange(angleDeg*M_PI/180,range);
    else {
	dbg("Lasers.setBackground",1) << "Point " << scanpt << " is at close range of " << range << " skipping it." << std::endl;
	background[scanpt]=background[(scanpt+totalpts-1)%totalpts];
    }
}

// Compute stats for allocation to lasers
std::vector<LaserStat> Lasers::computeStats(const Composite &c, const Ranges &ranges) const {
    const CPoints &points = c.getPoints();
    dbg("Composite.computeStats",1) << "Stats for composite " << c.getShapeID()->getID() << " with " << points.size() << " points near " << points.front() << ", prior laser=" << c.getShapeID()->getLaser()  << std::endl;
    std::vector<LaserStat> stats;
    for (int i=0;i<lasers.size();i++) {
	LaserStat stat;
	if (!lasers[i]->isEnabled()) {
	    stat.fracOnScreen=-1;
	    stat.fracShadowed=1;
	} else {
	    stat.fracOnScreen=lasers[i]->fracOnScreen(points);
	    stat.fracShadowed=lasers[i]->fracShadowed(points,ranges,LASERSEP, TARGETSEP);
	}
	stat.meanDistance=lasers[i]->meanDistance(points);
	dbg("Composite.computeStats",1) << "Laser " << i << " onScreen=" << (int)(stat.fracOnScreen*100) << "%, shadowed=" << (int)(stat.fracShadowed*100) << "%, dist=" << stat.meanDistance << " m." << std::endl;
	stats.push_back(stat);
    }
    return stats;
}
