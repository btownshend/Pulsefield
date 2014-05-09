#pragma once

#include "opencv2/core/core.hpp"
#include "point.h"
#include "etherdream.h"

class Color;

// Transformation between floor (meter) space and device (-32767:32767) space
class Transform {
    cv::Mat transform, invTransform; 
    std::vector<cv::Point2f> floorpts, devpts;
 public:
    Transform();
    void clear();
    // Set transform based on floor space coordinates of particular device space points
    void set(std::vector<Point> devPts, std::vector<Point> floorPts);
    // Mapping, if out-of-range, return clipped point
    etherdream_point mapToDevice(Point floorPt,const Color &c) const;
    // Inverse mapping from laser to world
    Point mapToWorld(etherdream_point p) const;

    std::vector<etherdream_point> mapToDevice(std::vector<Point> floorPts,Color c) const;
    std::vector<Point> mapToWorld(std::vector<etherdream_point> pts) const;

    // Computer transform matrix from set of points already provided
    void setTransform();
    void addToMap(Point devSpace, Point floorSpace) {
	floorpts.push_back(cv::Point2f(floorSpace.X(), floorSpace.Y()));
	devpts.push_back(cv::Point2f(devSpace.X(), devSpace.Y()));
    }
};

