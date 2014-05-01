#pragma once
#include <vector>
#include "etherdream.h"
#include "point.h"

class Laser {
    static const int MAXSLEWDISTANCE;  // Maximum distance mirrors can move in 1 point time
    static const int PPS;
    struct etherdream *d;
 public:
    Laser();
    int open();
    void update(const std::vector<etherdream_point> points);
    static std::vector<etherdream_point> getBlanks(etherdream_point initial, etherdream_point final);
};
