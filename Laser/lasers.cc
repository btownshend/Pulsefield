#include "lasers.h"

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

int Lasers::render() {
    if (!needsRender) {
	dbg("Lasers.render",5) << "Not dirty" << std::endl;
	return 0;
    }
    lock();
    needsRender=false;
    Drawing dtmp=drawing;
    const Color bgColor=Color(0.0,1.0,0.0);
    const Color gridColor=Color(0.0,1.0,0.0);
    const Color outlineColor=Color(0.0,1.0,0.0);

    if (showBackground)
	dtmp.drawPolygon(background,bgColor);
    if (showGrid) {
	float width=23*12/39.37;
	float depth=20*12/39.37;
	float minx=-width/2,maxx=width/2,miny=0,maxy=depth;
	float xstep=(maxx-minx)/4*0.999;
	for (float x=minx;x<=maxx;x+=xstep*2) {
	    dtmp.drawLine(Point(x,miny),Point(x,maxy),gridColor);
	    if (x+xstep<=maxx)  {
		dtmp.drawLine(Point(x,maxy),Point(x+xstep,maxy),gridColor);
		dtmp.drawLine(Point(x+xstep,maxy),Point(x+xstep,miny),gridColor);
	    }
	    if (x+2*xstep<=maxx)
		dtmp.drawLine(Point(x+xstep,miny),Point(x+2*xstep,miny),gridColor);
	}
	float ystep=(maxx-minx)/4*0.999;
	for (float y=miny;y<maxy;y+=ystep*2) {
	    dtmp.drawLine(Point(minx,y),Point(maxx,y),gridColor);
	    if (y+ystep<=maxy) {
		dtmp.drawLine(Point(maxx,y),Point(maxx,y+ystep),gridColor);
		dtmp.drawLine(Point(maxx,y+ystep),Point(minx,y+ystep),gridColor);
	    }
	    if (y+2*ystep<=maxy)
		dtmp.drawLine(Point(minx,y+ystep),Point(minx,y+2*ystep),gridColor);
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
	    std::vector<Point> outlineWorld=lasers[i]->getTransform().mapToWorld(outline);
	    dbg("Lasers.render",3) << "outlineWorld=";
	    for (int j=0;j<outlineWorld.size();j++)
		dbgn("Lasers.render",3) << outlineWorld[j] << " ";
	    dbgn("Lasers.render",3) << std::endl;
	    Drawing dtmp2=dtmp;
	    dtmp2.drawPolygon(outlineWorld,outlineColor);
	    lasers[i]->render(dtmp2);
	} else
	    lasers[i]->render(dtmp);
    }
    dbg("Lasers.render",1) << "Render done" << std::endl;
    unlock();
    return 1;
}

void Lasers::setDrawing(const Drawing &_drawing) {
    if (_drawing.getFrame() == drawing.getFrame()) {
	dbg("Lasers.setDrawing",1) << "Multiple drawings for same frame: " << drawing.getFrame() << std::endl;
    }
    lock();
    drawing=_drawing;
    needsRender=true;
    unlock();
    dbg("Lasers.setDrawing",2) << "Frame=" << drawing.getFrame() << std::endl;
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
