#pragma once

#include <iostream>
#include "opencv2/core/core.hpp"
#include "point.h"
#include "etherdream_bst.h"

class Color;

// Transformation between floor (meter) space and device (-32767:32767) space
class Transform {
    cv::Mat transform, invTransform; 
    std::vector<cv::Point2f> floorpts, devpts;
 public:
    Transform();
    void clear();

    // Mapping, if out-of-range, return clipped point
    etherdream_point mapToDevice(Point floorPt,const Color &c) const;
    Point mapToDevice(Point floorPt) const;

    // Inverse mapping from laser to world
    Point mapToWorld(etherdream_point p) const;

    std::vector<etherdream_point> mapToDevice(const std::vector<Point> &floorPts,Color c) const;
    std::vector<Point> mapToDevice(const std::vector<Point> &floorPts) const;
    std::vector<Point> mapToWorld(const std::vector<etherdream_point> &pts) const;

    // Compute transform matrix from set of points already provided
    void recompute();

    // Setup mapping
    // Point indices are order based on laser positions: BL, BR, TR, TL
    Point getFloorPoint(int i) const { assert(i>=0&&i<(int)floorpts.size()); return Point(floorpts[i].x,floorpts[i].y); }
    Point getDevPoint(int i) const { assert(i>=0&&i<(int)floorpts.size()); return Point(devpts[i].x,devpts[i].y); }
    void setFloorPoint(int i, Point floorpt) { assert(i>=0&&i<(int)floorpts.size()); floorpts[i].x=floorpt.X(), floorpts[i].y=floorpt.Y();  }
    void setDevPoint(int i, Point devpt) { assert(i>=0&&i<(int)floorpts.size()); devpts[i].x=devpt.X(); devpts[i].y=devpt.Y();  }

    // Load/save 
    void save(std::ostream &s) const;
    void load(std::istream &s);
};

