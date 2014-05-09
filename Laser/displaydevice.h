#pragma once
#include <vector>
#include "etherdream.h"

class DisplayDevice {
 public:
    DisplayDevice() {;}
    virtual int open() = 0;
    virtual void update(const std::vector<etherdream_point> points) = 0;
};
