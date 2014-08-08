#include <algorithm>
#include <assert.h>
#include "ranges.h"
#include "dbg.h"

static const float RANGEDEPTH=0.15f;   // Depth of a hit (distance behind a LIDAR still considered to be blocking something

Ranges::Ranges() {
    std::vector<float> r;
    r.assign(571,5.0f);   	// Default in case frontend not running
    setRanges(r);
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
    assert(isfinite(p.X()) && isfinite(p.Y()));
    float angle=p.getTheta();
    int scan=angleToScan(angle);
    if (scan<0)
	scan=0;
    else if (scan>=ranges.size())
	scan=ranges.size()-1;
    dbg("Ranges.pointToScan",5) << p << " -> " << scan << ", res=" << getScanRes()*180/M_PI << ", angle=" << angle*180/M_PI << std::endl;
    return scan;
}

// Return true if ray from p1 to p2 is obstructed by a target
bool Ranges::isObstructed(Point p1, Point p2, float p1gap, float p2gap) const {
    // Inset p1, p2 by their respective gaps
    Point dir=p2-p1; dir=dir/dir.norm();
    p1=p1+dir*p1gap;
    p2=p2-dir*p2gap;

    int scan1=pointToScan(p1);	
    int scan2=pointToScan(p2);
    assert(scan1>=0 && scan2>=0 && scan1<ranges.size() && scan2<ranges.size());
    if (scan1>scan2) 
	std::swap(scan1,scan2);
    float dx=p2.X()-p1.X();
    float dy=p2.Y()-p1.Y();
    float numerator=dy*p1.X()-dx*p1.Y();
    // Only consider scan directions that definitely intersect p1-p2 line
    for (int i=scan1+1;i<=scan2-1;i++) {
	if (ranges[i]<0.2)
	    // Probably dust on lens of lidar
	    continue;
	Point hit=getPoint(i);
	Point v=hit/ranges[i];
	// Calculate distance from LIDAR along scan line i to line connecting p1 and p2
	float denom=(dy*v.X()-dx*v.Y());
	if (denom==0)
	    // No intersection
	    continue;
	float d=numerator/denom;   // Distance along LIDAR scan to line connecting p1 and p2
	if (ranges[i]<=d && ranges[i]+RANGEDEPTH>=d) {
	    // Hit is likely obstructing laser line
	    dbg("Ranges.isObstructed",4) << "scan " << i << ": p1=" << p1 << ", p2=" << p2 << ", hit=" << hit << ", d=" << d << ", range=" << ranges[i] << " -> TRUE" << std::endl;
	    return true;
	}
    }
    return false;
}

// Fraction of line shadowed
// Center of view is at c, line goes from (p1,p2)
// Laser scans originate at (0,0)
float Ranges::fracLineShadowed(Point c, Point p1, Point p2) const {
    static const float lineRes=0.2;   // 20cm resolution
    const float SHADOWSEP=0.5f;   // Distance an obstruction must be in front of an object for it to be considered shadowed
    // Check each ray
    int nrays=std::max(int((p2-p1).norm()/lineRes)+1,2);
    //    nrays=4;
    int shadowed=0;
    for (int i=0;i<nrays;i++) {
	Point p=(p1*i+p2*(nrays-1-i))/(nrays-1);
	if (isObstructed(c,p,SHADOWSEP,SHADOWSEP))
		shadowed++;
    }
    dbg("Ranges.fracLineShadowed",4) << shadowed << "/" << nrays << " rays shadowed" << std::endl;
    return shadowed*1.0f/nrays;
};

void Ranges::setRanges(const std::vector<float> _ranges) {
    ranges=_ranges;
    points.resize(ranges.size());
    for (int i=0;i<ranges.size();i++)
	points[i].setThetaRange(getAngleRad(i),ranges[i]);
}
