#include <iostream>
#include <assert.h>
#include "cpoint.h"
#include "dbg.h"

std::ostream& operator<<(std::ostream &s, const CPoint &p) {
    s << (Point)p << p.c;
    return s;
}

std::ostream& operator<<(std::ostream &s, const std::vector<CPoint> &p) {
    s << "[ ";
    for (int i=0;i<p.size();i++)
	s << p[i] << " ";
    s << "]";
    return s;
}


std::vector<CPoint> CPoint::resample(std::vector<CPoint> pts, int npts) {
    if (pts.size()<=1)
	return pts;
    if (npts==-1)
	npts=pts.size();
    assert(npts>=2);
    std::vector<CPoint> result(npts);
    float totalLen=0;
    for (int i=1;i<pts.size();i++) {
	totalLen+=(pts[i]-pts[i-1]).norm();
    }
    float spacing=totalLen/(npts-1);
    float pos=0;
    float j=0;
    result[0]=pts[0];
    for (int i=1;i<result.size()-1;i++)  {
	while (pos < spacing*i) {
	    j++;
	    pos+=(pts[j]-pts[j-1]).norm();
	    assert(j<pts.size());
	}
	// New point is between old point j-1 and j
	float frac=1-(pos-(spacing*i))/(pts[j]-pts[j-1]).norm();   // Fraction of distance from j-1 to j
	result[i]=pts[j-1]*(1-frac)+pts[j]*frac;
	dbg("resample",1) << j << ": " << pts[j] << ", len=" << (pts[j-1]-pts[j]).norm() << " -> " << "frac=" << frac << ",i=" << i << ": " << result[i] << ", len=" << (result[i-1]-result[i]).norm() << std::endl;
    }
    result[npts-1]=pts[pts.size()-1];
    return result;
}
