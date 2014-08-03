#pragma once

#include <iostream>
#include "opencv2/core/core.hpp"
#include "point.h"
#include "cpoint.h"
#include "etherdream_bst.h"

class Color;

// Transformation between floor (meter) space, flat  device window [-1:1] and  device (-32767:32767) space
// Floor<-> Flat is a perspective transformation (imagine flat/normalized as a viewport perpendicular to the scanner, such as the physical window)
// Flat <-> Device has a cos() warping transformation :  fy=dy/32767/cos(dy/32767*vfov/2);   fx=dx/32767/cos(dy/32767*vfov/2)/cos(dx/32767*hfov/2)
// In addition, keep track of the physical window (the range of usable projection) in flat space, values between -1 and 1.  Use flat because the view window is approximately rectangular in this space.

class Transform {
    cv::Mat transform, invTransform; 
    std::vector<Point> floorpts, devpts;
    float hfov, vfov;  // Field of view corresponding to full device range around origin (e.g. [-32767,0]:[32767,0] covers hfov); assumes laser mirrors are centered for (0,0)
    static std::vector<cv::Point2f> convertPoints(const std::vector<Point> &pts);
    float minx, maxx, miny, maxy;   // Bounds of laser projection (in flat cooords)
    etherdream_point cPointToEtherdream(CPoint devPt) const;
    Point origin;   // Location of projector in floor coordinate space (with z-value ignored)
 public:
    Transform();
    void clear(float floorMinx, float floorMiny, float floorMaxx, float floorMaxy);

    // Set projector field of view (in Radians)
    void setHFOV(float hfov) { this->hfov=hfov; recompute(); }
    void setVFOV(float vfov) { this->vfov=vfov; recompute(); }
    float getHFOV() const { return hfov; }
    float getVFOV() const { return vfov; }

    // Mapping, if out-of-range, return clipped point
    etherdream_point mapToDevice(CPoint floorPt) const;
    Point mapToDevice(Point floorPt) const;

    // Inverse mapping from laser to world
    Point mapToWorld(Point p) const;
    CPoint mapToWorld(etherdream_point p) const;

    // Normalized space to world
    Point flatToWorld(Point flatPt) const;
    // Convert  device coord to flat space (i.e. laser projection grid)
    Point deviceToFlat(Point devPt) const;
    std::vector<Point> deviceToFlat(const std::vector<Point> &devPts) const;
    // Convert  flat space coord to device coords
    Point flatToDevice(Point flatPt) const;
    etherdream_point  flatToDevice(CPoint flatPt) const;
    std::vector<Point> flatToDevice(const std::vector<Point> &flatPts) const;

    std::vector<etherdream_point> mapToDevice(const std::vector<CPoint> &floorPts) const;
    std::vector<Point> mapToDevice(const std::vector<Point> &floorPts) const;
    std::vector<CPoint> mapToWorld(const std::vector<etherdream_point> &pts) const;

    // Check if a given device coordinate is "on-screen" (can be projected)
    bool onScreen(Point devPt) const;
    bool onScreen(etherdream_point devPt) const { return onScreen(Point(devPt.x,devPt.y)); }

    // Get/set min/max of device window (in normalized coords -1:1 )
    float getMinX() const { return minx; } 
    float getMaxX() const { return maxx; } 
    float getMinY() const { return miny; } 
    float getMaxY() const { return maxy; } 
    void setMinX(float minx)  { this->minx=minx; }
    void setMaxX(float maxx)  { this->maxx=maxx; }
    void setMinY(float miny)  { this->miny=miny; }
    void setMaxY(float maxy)  { this->maxy=maxy; }
    
    // Clip a line to the onScreen portion
    void clipLine(Point &floorPt1, Point &floorPt2) const;

    // Compute transform matrix from set of points already provided
    void recompute();

    // Setup mapping
    // Point indices are order based on laser positions: BL, BR, TR, TL
    Point getFloorPoint(int i) const { assert(i>=0&&i<(int)floorpts.size()); return floorpts[i]; }
    Point getDevPoint(int i) const { assert(i>=0&&i<(int)floorpts.size()); return devpts[i]; }
    void setFloorPoint(int i, Point floorpt) { assert(i>=0&&i<(int)floorpts.size()); floorpts[i]=floorpt; }
    void setDevPoint(int i, Point devpt) { assert(i>=0&&i<(int)floorpts.size()); devpts[i]=devpt; }

    // Load/save 
    void save(std::ostream &s) const;
    void load(std::istream &s);

    // Origin
    Point getOrigin() const { return origin; }
};

