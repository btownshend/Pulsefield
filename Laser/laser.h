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
    int unit;
 public:
    Laser(int unit);
    int open();
    void update();
    static std::vector<etherdream_point> getBlanks(etherdream_point initial, etherdream_point final);
    const std::vector<etherdream_point> &getPoints() const { return pts; }
    void setPoints(int _npoints) { npoints=_npoints; }
    void render(const Drawing &drawing);
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
    unsigned int size() const { return lasers.size(); }
};


