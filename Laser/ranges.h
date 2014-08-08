#pragma once
#include <vector>
#include "point.h"

class Ranges {
    int pointToScan(Point p) const;
    int angleToScan(float angleInRad) const;
    float getAngleRad(int i) const;
    std::vector<float> ranges;
    std::vector<Point> points;
public:
    Ranges();

    // Check if there is an obstruction (a LIDAR hit) along the line from p1 to p2, not including hits within gaps of p1 or p2
    bool isObstructed(Point p1, Point p2, float p1Gap, float p2Gap) const;

    void setRanges(const std::vector<float> _ranges);
    int size() const { return ranges.size(); }

    // Scan resolution in radians
    float getScanRes() const;

    // Compute amount of given structures are shadowed
    float fracLineShadowed(Point laser, Point p1, Point p2) const; // Fraction of line shadowed
    float fracCircleShadowed(Point laser, Point p1, float radius) const {	// Fraction of circle shadowed
	// Approx same as a line that is perpendicular to LIDAR scan line
	return fracLineShadowed(laser,p1+Point(radius,0),p1+Point(-radius,0));
    }

    Point getPoint(int i) const { return points[i]; }
};
