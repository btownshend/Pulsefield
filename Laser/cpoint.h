#pragma once

#include <iostream>
#include <vector>
#include "color.h"
#include "point.h"

class Bounds;

class CPoint: public Point {
    Color c;   // Color used to move to this point
 public:
    CPoint(): Point(), c(0,0,0) {;}
    CPoint(float _x, float _y,Color _c): Point(_x,_y), c(_c) {;}
    CPoint(const Point &p, Color _c): Point(p), c(_c) {;}
    CPoint rot90() const {
	return CPoint(Point::rot90(),c);
    }
    Color getColor() const { return c; }
    void setColor(Color _c) { c=_c; }
    CPoint operator-(const Point &p2) const { return CPoint((Point)*this-p2,c); }
    CPoint operator+(const Point &p2) const { return CPoint((Point)*this+p2,c); }
    CPoint operator/(float s) const { return CPoint((Point)*this / s,c); }
    CPoint operator*(float s) const { return CPoint((Point)*this *s,c); }
    CPoint operator-(float s) const { return CPoint((Point)*this -s,c); }
    CPoint operator+(float s) const { return CPoint((Point)*this+s,c); }
    friend std::ostream& operator<<(std::ostream &s, const CPoint &p);
    friend std::istream& operator>>(std::istream &s,  CPoint &p);
    CPoint min(const Point &p2) const { return CPoint(((Point *)this)->min(p2),c); }
    CPoint max(const Point &p2) const { return CPoint(((Point *)this)->max(p2),c); }
    bool operator==(const CPoint &p2) const { return ((Point)*this)==(Point)p2 && c==p2.getColor(); }
    // Interpolate between this point and another one; use the color of the second point for the result
    CPoint interpolate(const CPoint &p2, float frac) const { return CPoint((*this)*(1-frac)+p2*frac,p2.c); }

    static std::vector<Point> convertToPointVector(const std::vector<CPoint> &cpts) {
	std::vector<Point> result(cpts.size());
	for (int i=0;i<cpts.size();i++)
	    result[i]=cpts[i];
	return result;
    }
};

// A list of points indicating drawing segments
// Moves indicated with a destination point with color 0,0,0
// Can be in any coordinate space, but some operations assume that the points are in world (floor) coordinates
class CPoints {
    std::vector<CPoint> pts;
 public:
    CPoints(const std::vector<CPoint> &_pts) { pts=_pts; }
    CPoints() {;}
    const std::vector<CPoint> &getPoints() const { return pts; }
    unsigned int size() const { return pts.size(); } 
    void push_back(CPoint p) { pts.push_back(p); }
    // Resample
    CPoints resample(float spacing) const;
    float getLength() const {
	float len=0.0f;
	for (unsigned int i=0;i<pts.size();i++) {
	    if (pts[i].getColor()!=Color(0,0,0))
		len+=(pts[i]-pts[(i-1+pts.size())%pts.size()]).norm();
	}
	return len;
    }
    const CPoint &back() const { return pts.back(); }
    const CPoint &front() const { return pts.front(); }
    CPoint &front() { return pts.front(); }
    CPoints clip(const Bounds &b) const;
    void append(const CPoints &cp) {
	pts.insert(pts.end(),cp.pts.begin(),cp.pts.end());
    }
    friend std::ostream& operator<<(std::ostream &s, const CPoints &cp);
    CPoints convexHull(float spacing, Color c) const;
    CPoint &operator[](int i) { return pts[i]; }
    const CPoint &operator[](int i) const { return pts[i]; }
};
