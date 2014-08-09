#include <iostream>
#include <fstream>
#include <assert.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "cpoint.h"
#include "dbg.h"
#include "bounds.h"
#include "etherdream_bst.h"

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

CPoint::CPoint(const etherdream_point &ep): Point(ep.x,ep.y), c(ep.r/65535.0,ep.g/65535.0,ep.b/65535.0) {
    ;
}

CPoints::CPoints(const std::vector<etherdream_point> &epts) {
    pts.resize(epts.size());
    for (int i=0;i<epts.size();i++)
	pts[i]=epts[i];
}

void CPoints::matlabDump(std::string desc) {
    static std::ostream *ofd=NULL;
    if (ofd==NULL)  {
	ofd=new std::ofstream("/tmp/cpoint.m");
	*ofd << "cp={}; descs={};" << std::endl;
    }
    *ofd << "descs{end+1}='" << desc << "';" << std::endl;
    *ofd << "cp{end+1}=[";
    for (int i=0;i<pts.size();i++) {
	if (pts[i].getColor()==Color(0,0,0))
	    *ofd << "nan,nan,0" << std::endl;
	*ofd << pts[i].X() << "," << pts[i].Y()  << "," << pts[i].getColor().green() << std::endl;
    }
    *ofd << "];" << std::endl;
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
    int pendingBlank=-1;
    for (int i=0;i<pts.size();i++) {
	bool isBlank=(pts[i].getColor() == Color(0,0,0)) || !b.contains(pts[i]);
	if (isBlank)
	    pendingBlank=i;
	else {
	    if (pendingBlank >= 0) {
		CPoint ptmp=pts[pendingBlank];
		ptmp.setColor(Color(0,0,0));
		result.push_back(ptmp);
	    }
	    result.push_back(pts[i]);
	    pendingBlank=-1;
	}
    }
    // Don't need trailing blanks

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
