/*
 * point.h
 *
 *  Created on: Mar 30, 2014
 *      Author: bst
 */

#ifndef POINT_H_
#define POINT_H_

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
    float getTheta() const { return atan2(y,x)-M_PI/2; }
    void setThetaRange(float theta, float range) {
	x=cos(theta+M_PI/2)*range; 
	y=sin(theta+M_PI/2)*range;
    }
    Point operator-(const Point &p2) { return Point(x-p2.X(),y-p2.Y()); }
    Point operator+(const Point &p2) { return Point(x+p2.X(),y+p2.Y()); }
    Point operator/(float s) { return Point(x/s,y/s); }
    Point operator*(float s) { return Point(x*s,y*s); }
    float norm() const { return getRange(); }
    float dot(const Point &p2) const { return p2.X()*x+p2.Y()*y; }
};

#endif  /* POINT_H_ */
