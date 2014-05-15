#pragma once
#include "laser.h"
#include "drawing.h"

class Lasers {
    std::vector<std::shared_ptr<Laser> > lasers;
    Drawing drawing;
    std::vector<Point> background;   // Background
    bool showBackground,showGrid;

    bool needsRender;
    // Locking
    pthread_mutex_t mutex;
public:
    Lasers(int nunits);
    ~Lasers();
    int render();  // Refresh; return 1 if anything changed
    void setDrawing(const Drawing &_drawing);
    int getDrawingFrame() const { return drawing.getFrame(); }
    std::shared_ptr<Laser>  getLaser(int unit) { return lasers[unit]; }
    std::shared_ptr<const Laser> getLaser(int unit) const  { return lasers[unit]; }
    unsigned int size() const { return lasers.size(); }

    // Locking
    void lock();
    void unlock();

    // Save/load all transforms of all lasers
    void saveTransforms(std::ostream &s) const;
    void loadTransforms(std::istream &s);

    void setBackground(int scanpt, int totalpts, float angleDeg, float range);
    void toggleBackground() { showBackground=!showBackground; }
};


