#pragma once
#include "laser.h"
#include "drawing.h"
#include "ranges.h"

class Lasers {
    static std::shared_ptr<Lasers> theInstance;   // Singleton
    std::vector<std::shared_ptr<Laser> > lasers;
    std::vector<Point> background;   // Background

    // Visual for background
    std::shared_ptr<Drawing> visual;

    bool needsRender;
    // Locking
    pthread_mutex_t mutex;

    // Allocate to individual lasers
    std::vector<Drawing> allocate(const Drawing &d,const Ranges &ranges) const;

    // Current frame
    int frame;

    // Flags
    std::map<std::string, bool> flags;
    Lasers(int nunits);   // Only ever  called by initialize
public:
    static void initialize(int nunits) { assert(!theInstance); theInstance=std::shared_ptr<Lasers>(new Lasers(nunits)); }
    static std::shared_ptr<Lasers> instance() {
	assert(theInstance);
	return theInstance;
    }
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

    int render(const Ranges &ranges);    // Refresh; return 1 if anything changed
    void setFrame(int _frame) { frame=_frame; setDirty(); }
    void setDirty() { needsRender=true; }
    int getDrawingFrame() const { return frame; }
    std::shared_ptr<Laser>  getLaser(int unit) { if (unit<lasers.size()) return lasers[unit]; else return nullptr; }
    std::shared_ptr<const Laser> getLaser(int unit) const  { if (unit<lasers.size()) return lasers[unit]; else return nullptr; }
    unsigned int size() const { return lasers.size(); }

    // Locking
    void lock();
    void unlock();

    // Save/load all transforms of all lasers
    void saveTransforms(std::ostream &s) const;
    void loadTransforms(std::istream &s);
    // Clear transforms so that given real world bounds are at laser extents
    void clearTransforms(float floorMinx, float floorMiny, float floorMaxx, float floorMaxy);

    void setBackground(int scanpt, int totalpts, float angleDeg, float range);

    void setFlag(std::string flag, bool value) { flags[flag]=value; TouchOSC::instance()->send("/ui/laser/"+flag,value?1.0:0.0); }
    void toggleFlag(std::string flag) { setFlag(flag,!getFlag(flag)); }
    bool getFlag(std::string flag) { return flags[flag]; }

    void enable(int i, bool enable) { if (i>=0 && i<lasers.size()) lasers[i]->enable(enable); }
    bool isEnabled(int i) const { if (i>=0 && i<lasers.size()) return lasers[i]->isEnabled(); return false; }
    void toggleEnable(int i) { enable(i,!isEnabled(i)); }

    void dumpPoints() { for (int i=0;i<lasers.size();i++) lasers[i]->dumpPoints(); }

    void setVisual(const Drawing &d) {
	dbg("Lasers.setVisual",1) << "Set bg visual to drawing with " << d.getNumElements() << " elements." << std::endl;
	visual=std::shared_ptr<Drawing>(new Drawing(d));
    }
};
