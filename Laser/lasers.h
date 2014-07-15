#pragma once
#include "laser.h"
#include "drawing.h"

class Lasers {
    std::vector<std::shared_ptr<Laser> > lasers;
    std::vector<Point> background;   // Background
    bool showBackground,showGrid,showOutline,showTest,showAlignment;

    bool needsRender;
    // Locking
    pthread_mutex_t mutex;

    // Allocate to individual lasers
    std::vector<Drawing> allocate(const Drawing &d) const;

    // Current frame
    int frame;
public:
    Lasers(int nunits);
    ~Lasers();
    void setPoints(int npoints) {
	for (int i=0;i<lasers.size();i++) lasers[i]->setPoints(npoints);
	TouchOSC::instance()->send("/ui/laser/points/label",std::to_string(npoints)+" points");
    }
    void setSkew(int skew) {
	for (int i=0;i<lasers.size();i++) lasers[i]->setSkew(skew);
	TouchOSC::instance()->send("/ui/laser/skew/label",std::to_string(skew)+" skew");
    }
    void setPreBlanks(int n) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPreBlanks(n);
	TouchOSC::instance()->send("/ui/laser/preblank/label",std::to_string(n)+" pre-blank");
}
    void setPostBlanks(int n) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPostBlanks(n);
	TouchOSC::instance()->send("/ui/laser/postblank/label",std::to_string(n)+" post-blank");
    }
    void setPPS(int pps) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPPS(pps);
	TouchOSC::instance()->send("/ui/laser/pps/label",std::to_string(pps)+" PPS");
    }

    int render();  // Refresh; return 1 if anything changed
    void setFrame(int _frame) { frame=_frame; setDirty(); }
    void setDirty() { needsRender=true; }
    int getDrawingFrame() const { return frame; }
    std::shared_ptr<Laser>  getLaser(int unit) { return lasers[unit]; }
    std::shared_ptr<const Laser> getLaser(int unit) const  { return lasers[unit]; }
    unsigned int size() const { return lasers.size(); }

    // Locking
    void lock();
    void unlock();

    // Save/load all transforms of all lasers
    void saveTransforms(std::ostream &s) const;
    void loadTransforms(std::istream &s);
    void clearTransforms();

    void setBackground(int scanpt, int totalpts, float angleDeg, float range);
    void toggleBackground() { showBackground=!showBackground; }
    void toggleGrid() { showGrid=!showGrid; }
    void toggleAlignment() { showAlignment=!showAlignment; }
    void toggleOutline() { showOutline=!showOutline; }
    void toggleTest() { showTest=!showTest; }
    void toggleLaser(int i) { if (i>=0 && i<lasers.size()) lasers[i]->toggleLaser(); }
    void dumpPoints() { for (int i=0;i<lasers.size();i++) lasers[i]->dumpPoints(); }
};


