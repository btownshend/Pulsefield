#include <iostream>
#include <cmath>
#include "drawing.h"
#include "transform.h"
#include "laser.h"
#include "dbg.h"

std::ostream& operator<<(std::ostream& s, const Color &col) {
    return s << "[" << col.r << "," << col.g << "," << col.b << "]";
}

Color Color::getBasicColor(int i) {
    if (i==0)
	return Color(1.0,0.0,0.0);
    else if (i==1)
	return Color(0.0,1.0,0.0);
    else if (i==2)
	return Color(0.0,0.0,1.0);
    else if (i==3)
	return Color(0.5,0.5,0.0);
    else
	return Color(((i+1)%3)/2.0,((i+1)%5)/4.0,((i+1)%7)/6.0);
}

std::vector<etherdream_point> Circle::getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const {
    int npoints=std::ceil(getLength()/pointSpacing)+1;
    if (npoints < 5) {
	dbg("Circle.getPoints",1) << "Circle of radius " << radius << " with point spacing of " << pointSpacing << " only had " << npoints << " points; increasing to 5" << std::endl;
	npoints=5;
    }
    std::vector<etherdream_point> result(npoints);
    struct etherdream_point *pt = &result[0];
    float initphase;
    etherdream_point devcenter=transform.mapToDevice(center,c);
    if (priorPoint==0 || (priorPoint->x==devcenter.x && priorPoint->y==devcenter.y))
	initphase=0;
    else {
	// Find phase closest to prior point
	Point delta(priorPoint->x-devcenter.x,priorPoint->y-devcenter.y);
	initphase=atan2(delta.Y(),delta.X());
	dbg("Circle.getPoints",3) << "Delta=" << delta << ", initial phase = " << initphase << std::endl;
    }
    for (int i = 0; i < npoints; i++,pt++) {
	float phase = i * 2.0 * M_PI /(npoints-1)+initphase;
	*pt = transform.mapToDevice(Point(cos(phase) * radius + center.X(), sin(phase) * radius + center.Y()),c);
    }
    dbg("Circle.getPoints",2) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

std::vector<etherdream_point> Line::getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const {
    int npoints=std::max(2,(int)std::ceil(getLength()/pointSpacing)+1);
    bool swap=false;
    if (priorPoint!=NULL) {
	// Check if line direction should be swapped
	etherdream_point devp1=transform.mapToDevice(p1,c);
	etherdream_point devp2=transform.mapToDevice(p2,c);
	int d1=std::max(abs(priorPoint->x-devp1.x),abs(priorPoint->y-devp1.y));
	int d2=std::max(abs(priorPoint->x-devp2.x),abs(priorPoint->y-devp2.y));
	if (d2<d1) {
	    dbg("Line.getPoints",3) << "Swapping line endpoints; d1=" << d1 << ", d2=" << d2 << std::endl;
	    swap=true;
	}
    }
    std::vector<etherdream_point> result(npoints);
    for (int i = 0; i < npoints; i++) {
	float rpos=i*1.0/(npoints-1);
	if (swap) rpos=1-rpos;
	result[i] = transform.mapToDevice(p1+(p2-p1)*rpos,c);
    }
    dbg("Line.getPoints",2) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}


std::vector<etherdream_point> Cubic::getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const {
    std::vector<Point> pts = b.interpolate(pointSpacing);
    dbg("Cubic.getPoints",2) << "Converted to " << pts.size() << " points." << std::endl;
    return convert(pts,transform);
}

std::vector<etherdream_point> Primitive::convert(const std::vector<Point> &pts, const Transform &transform) const {
    std::vector<etherdream_point> result(pts.size());
    for (unsigned int i = 0; i < pts.size(); i++) {
	result[i] = transform.mapToDevice(pts[i],c);
    }
    return result;
}

std::vector<etherdream_point> Arc::getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const {
    assert(0);   // TODO
}


// Convert to points using given floorspace spacing
std::vector<etherdream_point> Drawing::getPoints(float spacing,const Transform &transform) const {
    dbg("Drawing.getPoints",2) << "getPoints(" << spacing << ")" << std::endl;
    std::vector<etherdream_point>  result;
    for (unsigned int i=0;i<elements.size();i++) {
	std::vector<etherdream_point> newpoints;
	if (result.size()>0)
	    newpoints = elements[i]->getPoints(spacing,transform,&result.back());
	else
	    newpoints = elements[i]->getPoints(spacing,transform,NULL);
	if (result.size()>0 && newpoints.size()>0)  {
	    // Insert blanks first
	    std::vector<etherdream_point> blanks = Laser::getBlanks(result.back(),newpoints.front());
	    result.insert(result.end(), blanks.begin(), blanks.end());
	}
	result.insert(result.end(), newpoints.begin(), newpoints.end());
    }
    // Add blanks for final skew back to start of figure
    if (result.size()>0) {
	std::vector<etherdream_point> blanks = Laser::getBlanks(result.back(),result.front());
	result.insert(result.end(), blanks.begin(), blanks.end());
    }
    dbg("Drawing.getPoints",2) << "Converted to " << result.size() << " points." << std::endl;
    for (unsigned int i=0;i<result.size();i++) {
	if (i==5) {
	    dbg("Drawing.getPoints",5)  << "...";
	    break;
	}
	dbg("Drawing.getPoints",5)  << "pt[" << i << "] = " << result[i].x << "," << result[i].y << " G=" << result[i].g << std::endl;
    }
    return result;
}

// Prune a sequence of points by removing any segments that go out of bounds
std::vector<etherdream_point> Drawing::prune(const std::vector<etherdream_point> pts) const {
    std::vector<etherdream_point> result;
    bool oobs=false;
    for (unsigned int i=0;i<pts.size();i++) {
	if (pts[i].x != -32768  && pts[i].x != 32767 && pts[i].y != -32768 && pts[i].y != 32767)  {
	    // In bounds
	    if (oobs && result.size()>0) {
		// Just came in bounds, insert blanks if needed
		std::vector<etherdream_point> blanks = Laser::getBlanks(result.back(),pts[i]);
		result.insert(result.end(), blanks.begin(), blanks.end());
	    }
	    result.push_back(pts[i]);
	    oobs=false;
	} else
	    oobs=true;
    }
    // May need end blanks (getBlanks will give an empty list if its not needed)
    if (result.size()>0) {
	std::vector<etherdream_point> blanks = Laser::getBlanks(result.back(),result.front());
	result.insert(result.end(), blanks.begin(), blanks.end());
    }

    dbg("Drawing.prune",2) << "Pruned points from " << pts.size() << " to " << result.size() << std::endl;
    return result;
}

// Convert drawing into a set of etherdream points
// Takes into account transformation to make all lines uniform brightness (i.e. separation of points is constant in floor dimensions)
std::vector<etherdream_point> Drawing::getPoints(int targetNumPoints,const Transform &transform, float &spacing) const {
    spacing=getLength()/targetNumPoints;
    std::vector<etherdream_point> result = getPoints(spacing,transform);
    std::vector<etherdream_point> pruned = prune(result);
    dbg("Drawing.getPoints",2) << "Initial point count = " << pruned.size() << " compared to planned " << targetNumPoints << " for " << elements.size() << " elements." << std::endl;
    if (std::abs(targetNumPoints-(int)pruned.size()) > 10) {
	int nblanks = 0;
	for (unsigned int i=0;i<pruned.size();i++)
	    if (pruned[i].r==0 && pruned[i].g==0 && pruned[i].b==0)
		nblanks++;
	if (nblanks> targetNumPoints/2)  {
	    targetNumPoints=nblanks*2;
	    dbg("Drawing.getPoints",2) << "More than half of points are blanks, increasing target to " << targetNumPoints << std::endl;
	}
	float scaleFactor=(targetNumPoints-nblanks)*1.0/(pruned.size()-nblanks);
	dbg("Drawing.getPoints",2) << "Have " << nblanks << " blanks; adjusting spacing by a factor of " << scaleFactor << std::endl;
	spacing/=scaleFactor;
	result = getPoints(spacing,transform);
	pruned = prune(result);
	dbg("Drawing.getPoints",2) << "Revised point count = " << pruned.size() << " compared to planned " << targetNumPoints << " for " << elements.size() << " elements." << std::endl;
    }
    return pruned;
}

