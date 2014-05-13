#pragma once
#include <vector>
#include "etherdream.h"
#include "transform.h"

class DisplayDevice {
 protected:
    Transform transform;	// 3x3 transformation matrix; maps floor position in meters to device coords
 public:
    DisplayDevice() {;}
    virtual ~DisplayDevice() {;}
    virtual int open() = 0;

    //    void setTransform(const Transform &t) {
    //	transform=t;
    // }

    const Transform &getTransform() const { return transform; }
    Transform &getTransform()  { return transform; }
};
