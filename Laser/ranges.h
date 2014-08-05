#pragma once
#include <vector>
#include "point.h"

class Ranges {
    static const float SHADOWSEP;   // Distance an obstruction must be in front of an object for it to be considered shadowed
    int pointToScan(Point p) const;
    int angleToScan(float angleInRad) const;
    float getAngleRad(int i) const;
    bool isObstructed(Point p1, Point p2) const;
public:
    Ranges();
    std::vector<float> ranges;
    int size() const { return ranges.size(); }
    // Compute amount of given structures are shadowed
    float getScanRes() const;
    float fracLineShadowed(Point laser, Point p1, Point p2) const; // Fraction of line shadowed
    float fracCircleShadowed(Point laser, Point p1, float radius) const {	// Fraction of circle shadowed
	// Approx same as a line that is perpendicular to LIDAR scan line
	return fracLineShadowed(laser,p1+Point(radius,0),p1+Point(-radius,0));
    }
    Point getPoint(int i) const;
};
