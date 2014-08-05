#include "lasers.h"
#include "connections.h"
#include "person.h"
#include "groups.h"
#include "video.h"

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
std::vector<Drawing> Lasers::allocate(const Drawing &d, const Ranges &ranges)  const {
    std::map<int,float> bestScores;
    std::map<int,int> bestlasers;
    bool firstLaser=true;
    for (unsigned int i=0;i<lasers.size(); i++) {
	if (lasers[i]->isEnabled()) {
	    std::map<int,float> scores=d.getShapeScores(lasers[i]->getTransform(),ranges);
	    for (std::map<int,float>::iterator s=scores.begin();s!=scores.end();s++) {
		if (firstLaser || s->second > bestScores[s->first]) {
		    bestScores[s->first]=s->second;
		    bestlasers[s->first]=i;
		}
	    }
	    firstLaser=false;
	}
    }
    std::vector<Drawing> result(lasers.size());
    for (unsigned int i=0;i<result.size(); i++)  {
	std::set<int> select;
	for (std::map<int,int>::iterator b=bestlasers.begin();b!=bestlasers.end();b++) {
	    if (b->second==i)
		select.insert(b->first);
	}
	result[i]=d.select(select);
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
	dbg("Lasers.render",1) << "After adding allocation test pattern, have " << drawing.getNumElements() << " elements." << std::endl;
    }

    // Split drawing among lasers
    std::vector<Drawing> dtmp=allocate(drawing,ranges);

    // Global drawing - applies to all lasers 
    Drawing globalDrawing;

    const Color bgColor=Color(0.0,1.0,0.0);
    const Color gridColor=Color(0.0,1.0,0.0);

    if (getFlag("background") && ranges.size()>0) {
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
    }
    if (getFlag("grid")) {
	int ngrid=7;
	// Assumes active region is 7m x 14m
	float width=13;
	float depth=6;
	float minx=-width/2+0.5f,maxx=width/2-0.5f,miny=0.5f,maxy=depth-0.5f;
	float xstep=(maxx-minx)/(ngrid-1);
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
    }
    for (unsigned int i=0;i<lasers.size();i++) {
	if (getFlag("outline") && lasers[i]->isEnabled()) {
	    lasers[i]->showOutline(bounds);
	}
	else if (getFlag("test") && lasers[i]->isEnabled()) {
	    lasers[i]->showTest();
	} else {
	    dtmp[i].append(globalDrawing);
	    lasers[i]->render(dtmp[i],bounds);
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
