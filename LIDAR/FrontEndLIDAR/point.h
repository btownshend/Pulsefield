/*
 * point.h
 *
 *  Created on: Mar 30, 2014
 *      Author: bst
 */

#ifndef POINT_H_
#define POINT_H_

#include <ostream>
#include <vector>
#include <math.h>

class Point {
    float x,y;
 public:
    Point() {x=0;y=0;}
    Point(float _x, float _y) { x=_x; y=_y; };
    float X() const { return x; }
    float Y() const { return y; }
    void setX(float _x) { x=_x; }
    void setY(float _y) { y=_y; }
    float getRange() const { return sqrt(x*x+y*y); }
    float getTheta() const {
	float th=atan2(y,x)-M_PI/2;
	if (th<-M_PI)
	    th+=2*M_PI;
	return th;
    }
    void setThetaRange(float theta, float range) {
	x=cos(theta+M_PI/2)*range; 
	y=sin(theta+M_PI/2)*range;
    }
    Point operator-(const Point &p2) const { return Point(x-p2.X(),y-p2.Y()); }
    Point operator+(const Point &p2) const { return Point(x+p2.X(),y+p2.Y()); }
    Point operator/(float s) const { return Point(x/s,y/s); }
    Point operator*(float s) const { return Point(x*s,y*s); }
    Point operator-(float s) const { return Point(x-s,y-s); }
    Point operator+(float s) const { return Point(x+s,y+s); }
    float norm() const { return getRange(); }
    float dot(const Point &p2) const { return p2.X()*x+p2.Y()*y; }
    friend std::ostream& operator<<(std::ostream &s, const Point &p);
    Point min(const Point &p2) const { return Point(std::min(x,p2.X()),std::min(y,p2.Y())); }
    Point max(const Point &p2) const { return Point(std::max(x,p2.X()),std::max(y,p2.Y())); }
};

// TODO Don't really belong here
inline std::ostream& operator<< (std::ostream& os, const std::vector<int>& v) 
{
    os << "[";
    for (std::vector<int>::const_iterator ii = v.begin(); ii != v.end(); ++ii) {
        os << " " << *ii;
    }
    os << " ]";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, const std::vector<float>& v) 
{
    os << "[";
    for (std::vector<float>::const_iterator ii = v.begin(); ii != v.end(); ++ii) {
        os << " " << *ii;
    }
    os << " ]";
    return os;
}

#endif  /* POINT_H_ */
