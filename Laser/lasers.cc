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
    if (showBackground)
	dtmp.drawPolygon(background,Color(1.0,1.0,1.0));
    for (unsigned int i=0;i<lasers.size();i++)
	lasers[i]->render(dtmp);
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
    background[scanpt].setThetaRange(angleDeg*M_PI/180,range);
}
