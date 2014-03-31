/*
 * target.h
 *
 *  Created on: Mar 30, 2014
 *      Author: bst
 */

#ifndef TARGET_H_
#define TARGET_H_

#include <vector>
#include "point.h"

class Target {
    unsigned int c;
    std::vector<Point> points;
    bool leftshadow,rightshadow;
    Point priorpt, nextpt;
 public:
    Target(unsigned int _c, const std::vector<Point> &p, bool l, bool r, Point ppt, Point npt) {
	c=_c;
	points=p;
	leftshadow=l;
	rightshadow=r;
	priorpt=ppt;
	nextpt=npt;
    }
    Point getCenter() const;
    unsigned int getClass() const { return c; }
    const std::vector<Point> &getPoints() const { return points; }
    Point getPriorPoint() const { return priorpt; }
    Point getNextPoint() const { return nextpt; }
    bool isLeftShadowed() const { return leftshadow; }
    bool isRightShadowed() const { return rightshadow; }
};

typedef std::vector<Target> Targets;

#endif  /* TARGET_H_ */
