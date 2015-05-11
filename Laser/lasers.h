#pragma once
#include "laser.h"
#include "drawing.h"
#include "ranges.h"

class Video;

// Stats for imaging an object with a particular laser
class LaserStat {
 public:
    float fracOnScreen;		// Fraction of object in laser's field of view
    float fracShadowed;	// Fraction of object shadowed by other hits
    float meanDistance;	// Mean distance from laser to object
};

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
    std::vector<Drawing> allocate(Drawing &d,const Ranges &ranges) const;

    // Current frame
    int frame;

    // Flags
    std::map<std::string, bool> flags;
    Lasers(int nunits,bool simulate);   // Only ever  called by initialize

    // Configuration file
    Configuration config;
public:
    static void initialize(int nunits, bool simulate) { assert(!theInstance); theInstance=std::shared_ptr<Lasers>(new Lasers(nunits,simulate)); }
    static std::shared_ptr<Lasers> instance() {
	assert(theInstance);
	return theInstance;
    }
    ~Lasers();
    void setNPoints(int npoints) {
	for (int i=0;i<lasers.size();i++) lasers[i]->setNPoints(npoints);
    }
    void setSkew(int skew) {
	for (int i=0;i<lasers.size();i++) lasers[i]->setSkew(skew);
    }
    void setIntensityPts(int intensityPts) {
	for (int i=0;i<lasers.size();i++) lasers[i]->setIntensityPts(intensityPts);
    }
    void setPreBlanks(int n) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPreBlanks(n);
}
    void setPostBlanks(int n) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPostBlanks(n);
    }
    void setPPS(int pps) { 
	for (int i=0;i<lasers.size();i++) lasers[i]->setPPS(pps);
    }

    int render(const Ranges &ranges, const Bounds &bounds);    // Refresh; return 1 if anything changed
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
    void save() const;
    void load();

    // Clear transforms so that given real world bounds are at laser extents
    void clearTransforms(const Bounds &floorBounds);

    void setBackground(int scanpt, int totalpts, float angleDeg, float range);

    void setFlag(std::string flag, bool value) { flags[flag]=value; }
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
    void clearVisuals() {
	visual.reset();
    }

    // Compute stats for allocation to lasers
    std::vector<LaserStat> computeStats(const Composite &c, const Ranges &ranges) const;
};
