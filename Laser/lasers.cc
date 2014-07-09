#include "lasers.h"
#include "connections.h"
#include "person.h"
#include "groups.h"

Lasers::Lasers(int nlasers): lasers(nlasers) {
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
    showBackground=false;
    showGrid=false;
}

Lasers::~Lasers() {
    (void)pthread_mutex_destroy(&mutex);
}

// Allocate a drawing to the lasers
std::vector<Drawing> Lasers::allocate(const Drawing &d)  const {
    std::map<int,float> bestScores;
    std::map<int,int> bestlasers;
    for (unsigned int i=0;i<lasers.size(); i++) {
	std::map<int,float> scores=d.getShapeScores(lasers[i]->getTransform());
	for (std::map<int,float>::iterator s=scores.begin();s!=scores.end();s++) {
	    if (i==0 || s->second > bestScores[s->first]) {
		bestScores[s->first]=s->second;
		bestlasers[s->first]=i;
	    }
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

int Lasers::render() {
    if (!needsRender) {
	dbg("Lasers.render",5) << "Not dirty" << std::endl;
	return 0;
    }
    lock();
    needsRender=false;

    Drawing drawing;
    Connections::draw(drawing);
    People::draw(drawing);
    Groups::draw(drawing);
    dbg("Lasers.render",1) << "People+Connections+Groups have " << drawing.getNumElements() << " elements." << std::endl;

    // Split drawing among lasers
    std::vector<Drawing> dtmp=allocate(drawing);

    // Global drawing - applies to all lasers 
    Drawing globalDrawing;

    const Color bgColor=Color(0.0,1.0,0.0);
    const Color gridColor=Color(0.0,1.0,0.0);
    const Color outlineColor=Color(0.0,1.0,0.0);

    if (showBackground)
	globalDrawing.drawPolygon(background,bgColor);
    if (showGrid) {
	int ngrid=7;
	float width=6;
	float depth=3;
	float minx=-width/2,maxx=width/2,miny=0,maxy=depth;
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
	if (showOutline) {
	    std::vector<etherdream_point> outline(5);
	    // Need to be inside maximums by at least one pixel or pruning will think its saturated and take it out
	    outline[0].x=-32766;outline[0].y=-32766;
	    outline[1].x=-32766;outline[1].y=32766;
	    outline[2].x=32766;outline[2].y=32766;
	    outline[3].x=32766;outline[3].y=-32766;
	    outline[4].x=-32766;outline[4].y=-32766;
	    // Convert to world 
	    std::vector<Point> outlineWorld=CPoint::convertToPointVector(lasers[i]->getTransform().mapToWorld(outline));
	    dbg("Lasers.render",3) << "outlineWorld=";
	    for (int j=0;j<outlineWorld.size();j++)
		dbgn("Lasers.render",3) << outlineWorld[j] << " ";
	    dbgn("Lasers.render",3) << std::endl;
	    dtmp[i].drawPolygon(outlineWorld,outlineColor);
	}
	if (showTest) {
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

void Lasers::clearTransforms() {
    for (unsigned int i=0;i<lasers.size();i++)
	lasers[i]->getTransform()=Transform();
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
