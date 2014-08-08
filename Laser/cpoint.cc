#include <iostream>
#include <assert.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "cpoint.h"
#include "dbg.h"
#include "bounds.h"

std::ostream& operator<<(std::ostream &s, const CPoint &p) {
    s << (Point)p << p.c;
    return s;
}

std::ostream& operator<<(std::ostream &s, const CPoints &p) {
    s << "[ ";
    for (int i=0;i<p.pts.size();i++)
	s << p.pts[i] << " ";
    s << "]";
    return s;
}


// Resample a set of points
// Assumes input has a single blank at each jump (effectively a move-to op)
// May truncate up to spacing units off end of each segment (resulting segment lengths are a multiple of spacing)
CPoints CPoints::resample(float spacing) const {
    float relpos=0;
    CPoints result;
    for (int j=0;j<pts.size();) {
	// Invariants:  relpos is the distance from the last point at which we should emit the next point
	// If possible emit a point using interpolation between input points j-1 and j
	if (pts[j].getColor()==Color(0,0,0)) {
	    // next point is blank, jump to it without interpolating path from last segment to this blank
	    // TODO - segments that are shorter than 'spacing' will be deleted.
	    result.push_back(pts[j]);
	    relpos=spacing; // Now move along new segment
	    j++;
	    continue;
	}
	float sep=(pts[j]-pts[j-1]).norm();
	if (relpos<=sep) {
	    // Normal point, interpolate from prior point
	    float frac;
	    if (sep>0)
		frac=relpos/sep;
	    else
		frac=1;
	    result.push_back(pts[j-1].interpolate(pts[j],frac));
	    relpos+=spacing;
	} else {
	    // Move to next point
	    relpos=relpos-sep;
	    j++;
	}
    }
    dbg("CPoints.resample",5) << "Resample(" << spacing << ")" << " converted " << pts.size() << " points into " << result.size() << std::endl;
    dbg("CPoints.resample",5) << "\tInput : " << pts << std::endl;
    dbg("CPoints.resample",5) << "\tResult: " << result << std::endl;
    return result;
}

CPoints CPoints::clip(const Bounds &b) const {
    CPoints result;
    bool wasBlank=false;
    for (int i=0;i<pts.size();i++) {
	bool isBlank=(pts[i].getColor() == Color(0,0,0)) || !b.contains(pts[i]);
	if (wasBlank && !isBlank) {
	    result.push_back(CPoint(pts[i],Color(0,0,0)));  // Move to new point with blanking
	}
	if (!isBlank)
	    result.push_back(pts[i]);
	//	dbg("CPoints.clip",4) << "i=" << i << ", wasBlank=" << wasBlank << ", isBlank=" << isBlank << ", result.size=" << result.size() << std::endl;
	wasBlank=isBlank;
    }
    if (wasBlank)
	// Make sure we get the final blank
	result.push_back(CPoint(pts.back(),Color(0,0,0)));
    dbg("CPoints.clip",3) << "Clipped drawing from " << pts.size() << " to " << result.size() << " points." << std::endl;
    return result;
}

CPoints CPoints::convexHull(float spacing, Color c) const {
	dbg("CPoints.convexHull",3) << "Converting into a convex hull" << std::endl;
	std::vector<cv::Point2f> src(pts.size());
	for (int i=0;i<pts.size();i++) {
	    src[i].x=pts[i].X();
	    src[i].y=pts[i].Y();
	}
	std::vector<cv::Point2f> dst;
	cv::convexHull(src,dst);
	dbg("CPoints.convexHull",3) << "Converted " << pts.size() << " points into a convex hull of size " << dst.size() << std::endl;
	CPoints result;
	for (int i=0;i<dst.size();i++) {
	    result.push_back(CPoint(dst[i].x,dst[i].y,(i==0)?Color(0,0,0):c));
	}
	result=result.resample(spacing);
	return result;
}
