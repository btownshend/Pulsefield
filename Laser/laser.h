#pragma once
#include <vector>
#include "etherdream.h"
#include "point.h"

class Laser {
    static int startBlank;   	// Points to blank at the beginning of each object
    static const int PPS=30000;
    std::vector<etherdream_point> points;
    struct etherdream *d;
 public:
    Laser();
    int open();
    void update();
    void setPoints(const std::vector<etherdream_point> &_points);
    void drawCircle(Point center, float r, int npts);
    void clear();
};
