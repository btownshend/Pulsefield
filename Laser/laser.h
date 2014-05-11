#pragma once

#include <vector>
#include "etherdream.h"
#include "displaydevice.h"
#include "drawing.h"

class Laser: public DisplayDevice  {
    int PPS;
    int npoints;
    struct etherdream *d;
    std::vector<etherdream_point> pts;
    float spacing;  // Separation in world coordinates of point spacing
    int unit;
    Color labelColor;
    Color maxColor;
 public:
    Laser(int unit);
    int open();
    void update();
    static std::vector<etherdream_point> getBlanks(etherdream_point initial, etherdream_point final);
    const std::vector<etherdream_point> &getPoints() const { return pts; }
    float getSpacing() const { return spacing; }
    void setPoints(int _npoints) { npoints=_npoints; }
    void render(const Drawing &drawing);
    Color getLabelColor() const { return labelColor; }
    Color getMaxColor() const { return maxColor; }
    int getUnit() const { return unit; }
};

class Lasers {
    std::vector<Laser*> lasers;
    Drawing drawing;
public:
    Lasers(int nunits);
    ~Lasers();
    void refresh();
    void render(const Drawing &_drawing) { drawing=_drawing; refresh(); }
    Laser *getLaser(int unit) { return lasers[unit]; }
    const Laser *getLaser(int unit) const { return lasers[unit]; }
    unsigned int size() const { return lasers.size(); }

    // Save/load all transforms of all lasers
    void saveTransforms(std::ostream &s) const;
    void loadTransforms(std::istream &s);
};


