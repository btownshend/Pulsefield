#pragma once

#include <vector>
#include "etherdream_bst.h"
#include "displaydevice.h"
#include "color.h"

class Drawing;

class Laser: public DisplayDevice  {
    int PPS;
    int npoints;
    struct etherdream *d;
    std::vector<etherdream_point> pts;
    float spacing;  // Separation in world coordinates of point spacing
    int unit;
    Color labelColor;
    Color maxColor;
    bool showLaser;
 public:
    Laser(int unit);
    int open();
    void update();
    static std::vector<etherdream_point> getBlanks(int blanks, etherdream_point pos);
    static std::vector<etherdream_point> getBlanks(etherdream_point initial, etherdream_point final);
    const std::vector<etherdream_point> &getPoints() const { return pts; }
    float getSpacing() const { return spacing; }
    void setPoints(int _npoints) { npoints=_npoints; }
    void render(const Drawing &drawing);
    Color getLabelColor() const { return labelColor; }
    Color getMaxColor() const { return maxColor; }
    int getUnit() const { return unit; }
    void toggleLaser() { showLaser=!showLaser; }
};
