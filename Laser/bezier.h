#include "point.h"

class Bezier {
    std::vector<Point> controlPoints;
    float length;
 public:
    Bezier(const std::vector<Point> &ctlPoints) { controlPoints=ctlPoints; length=-1; }

    // Find one point at particular fraction along curve
    Point getPoint(float t) const; 

    // Return a set of n points along a bezier curve sampled with uniform t (not uniformly spaced)
    std::vector<Point> interpolate(int n) const;
    // Return a set of n points along a bezier curve sampled a given t
    std::vector<Point> interpolate(std::vector<float> t) const;
    // Return a uniform set of points along a bezier curve with approximate spacing as given
    std::vector<Point> interpolate(float spacing) const;

    // Get length along path
    float getLength(float res=0.01) const;

    void translate(Point t) { 
	for (int i=0;i<controlPoints.size();i++)
	    controlPoints[i]=controlPoints[i]+t;
    }
};
