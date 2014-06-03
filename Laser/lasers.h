#pragma once
#include "laser.h"
#include "drawing.h"

class Lasers {
    std::vector<std::shared_ptr<Laser> > lasers;
    std::vector<Point> background;   // Background
    bool showBackground,showGrid,showOutline;

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
    int render();  // Refresh; return 1 if anything changed
    void setDirty(int _frame) { frame=_frame; needsRender=true; }
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
    void toggleOutline() { showOutline=!showOutline; }
    void toggleLaser(int i) { if (i>=0 && i<lasers.size()) lasers[i]->toggleLaser(); }
    void dumpPoints() { for (int i=0;i<lasers.size();i++) lasers[i]->dumpPoints(); }
};


