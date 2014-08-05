#include <algorithm>
#include <assert.h>
#include "ranges.h"
#include "dbg.h"

const float Ranges::SHADOWSEP=0.5f;   // Distance an obstruction must be in front of an object for it to be considered shadowed

Ranges::Ranges() {
    ranges.assign(571,5.0f);   	// Default in case frontend not running
}

float Ranges::getScanRes() const {
    const float scanRes=190.0/(ranges.size()-1)*M_PI/180.0;
    return scanRes;
}

int Ranges::angleToScan(float angleInRad) const {
    int scan=(ranges.size()-1)/2 + (int)(angleInRad/getScanRes() );
    return scan;
}

// Range angles go from -95 (i=0, [+x,0]) to 0 deg (i=191, [0,+y]) to 95 deg (i=381, [-x,0])
float Ranges::getAngleRad(int i) const {
    float angle= (i-(ranges.size()-1)/2.0f)*getScanRes();
    //    dbg("Ranges.getAngleRad",1) << "angle=" << angle << ", i=" << i << ", ranges.size()=" << ranges.size() << ", scanRes=" << scanRes << ", expr1=" << (i-(ranges.size()-1)/2.0f) << ", expr2=" <<  (i-(ranges.size()-1)/2)*scanRes <<std::endl;
    return angle;
}

// Find which scan line hits closest to point
int Ranges::pointToScan(Point p) const {
    float angle=p.getTheta();
    int scan=angleToScan(angle);
    if (scan<0)
	scan=0;
    else if (scan>=ranges.size())
	scan=ranges.size()-1;
    dbg("Ranges.pointToScan",5) << p << " -> " << scan << ", res=" << getScanRes()*180/M_PI << ", angle=" << angle*180/M_PI << std::endl;
    return scan;
}

Point Ranges::getPoint(int i) const {
    Point p;
    p.setThetaRange(getAngleRad(i),ranges[i]);
    return p;
}

// Return true if ray from p1 to p2 is obstructed by a target
bool Ranges::isObstructed(Point p1, Point p2) const {
    static const float RANGEDEPTH=0.3f;   // Depth of a hit
    int scan1=pointToScan(p1);
    int scan2=pointToScan(p2);
    assert(scan1>=0 && scan2>=0 && scan1<ranges.size() && scan2<ranges.size());
    if (scan1>scan2) 
	std::swap(scan1,scan2);
    float lineTheta=(p2-p1).getTheta();
    for (int i=scan1;i<=scan2;i++) {
	Point hit1,hit2;
	float angle=getAngleRad(i);
	hit1.setThetaRange(angle,ranges[i]);
	hit2.setThetaRange(angle,ranges[i]+RANGEDEPTH);
	// Check if the hit straddles the line
	if (((hit1-p1).getTheta()>lineTheta) != ((hit2-p1).getTheta()>lineTheta)) 
	    // And check if it is closer than the image we're trying to build
	    if ((hit1-p1).norm() < (p2-p1).norm()-SHADOWSEP) {
		dbg("Ranges.isObstructed",4) << "p1=" << p1 << ", p2=" << p2 << ", lineTheta=" << lineTheta << ", hit1=" << hit1 << ", hit2=" << hit2 << ", theta1=" << (hit1-p1).getTheta() << ", theta2=" << (hit2-p1).getTheta() << " -> TRUE" << std::endl;
		return true;
	    }
	//	dbg("Ranges.isObstructed",4) << "i=" << i << ", p1=" << p1 << ", p2=" << p2 << ", lineTheta=" << lineTheta << ", hit1=" << hit1 << ", hit2=" << hit2 << ", theta1=" << (hit1-p1).getTheta() << ", theta2=" << (hit2-p1).getTheta() << ", range=" << ranges[i] << ", angle=" << angle << " -> NO" << std::endl;
    }
    return false;
}

// Fraction of line shadowed
// Center of view is at c, line goes from (p1,p2)
// Laser scans originate at (0,0)
float Ranges::fracLineShadowed(Point c, Point p1, Point p2) const {
    static const float lineRes=0.1;   // 10cm resolution
    // Check each ray
    int nrays=int((p2-p1).norm()/lineRes)+1;
    nrays=4;
    int shadowed=0;
    for (int i=0;i<nrays;i++) {
	Point p=(p1*i+p2*(nrays-1-i))/(nrays-1);
	if (isObstructed(c,p))
		shadowed++;
    }
    dbg("Ranges.fracLineShadowed",4) << shadowed << "/" << nrays << " rays shadowed" << std::endl;
    return shadowed*1.0f/nrays;
};

