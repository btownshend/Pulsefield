#pragma once

#include <vector>
#include "color.h"
#include "point.h"

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
    
    static std::vector<Point> convertToPointVector(const std::vector<CPoint> &cpts) {
	std::vector<Point> result(cpts.size());
	for (int i=0;i<cpts.size();i++)
	    result[i]=cpts[i];
	return result;
    }
    // Equalize spacing by resampling
    static std::vector<CPoint> resample(std::vector<CPoint> pts, int npts=-1);
};
