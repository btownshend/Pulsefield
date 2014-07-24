#include "bezier.h"
#include "dbg.h"

// Find one point at particular fraction along curve
Point Bezier::getPoint(float t) const {
    std::vector<Point> tmp=controlPoints;

    int i = tmp.size() - 1;
    while (i > 0) {
        for (int k = 0; k < i; k++)
            tmp[k] = tmp[k] + ( tmp[k+1] - tmp[k] )*t;
        i--;
    }
    dbg("Bezier.getPoint",20) << "getPoint(" << t << ") -> " << tmp[0] << std::endl;
    return tmp[0];
}

// Return a set of n points along a bezier curve sampled with uniform t (not uniformly spaced)
std::vector<Point> Bezier::interpolate(int n) const {
    std::vector<Point> result(n);
    for (int i=0;i<n;i++)
	result[i]=getPoint(i*1.0/(n-1));
    return result;
}

// Return a set of n points along a bezier curve sampled a given t
std::vector<Point> Bezier::interpolate(std::vector<float> t) const {
    std::vector<Point> result(t.size());
    for (unsigned int i=0;i<result.size();i++)
	result[i]=getPoint(t[i]);
    return result;
}

// Return a uniform set of points along a bezier curve with approximate spacing as given
std::vector<Point> Bezier::interpolate(float spacing) const {
    if (spacing>getLength()*1.5) {
	dbg("Bezier.interpolate",1) << "Curve with length " << getLength() << " has no points for interpolation at spacing=" << spacing << std::endl;
	return std::vector<Point>();
    }
    if (spacing>getLength()*0.5) {
	std::vector<Point> result;
	result.push_back(controlPoints.front());
	result.push_back(controlPoints.back());
	dbg("Bezier.interpolate",1) << "Curve with length " << getLength() << " has 2 points for interpolation at spacing=" << spacing << std::endl;
	return result;
    }
    int npoints=std::max(2,(int)ceil(getLength()/spacing)+1);
    spacing=getLength()/(npoints-1);

    // Build initial set of positions
    std::vector<float> t(npoints);
    for (int i=0;i<npoints;i++)
	t[i]=i*1.0/(npoints-1);

    for (int iter=0;iter<3;iter++) {
	// Interpolate at these
	std::vector<Point> pts=interpolate(t);
	// Find better positions along curve

	// Find cumulative distance along path
	std::vector<float> cumdist(npoints);
	cumdist[0]=0;
	dbg("Bezier.interpolate",18) << "Cum dist=[0";
	for (int i=1;i<npoints;i++) {
	    cumdist[i]=cumdist[i-1]+(pts[i]-pts[i-1]).norm();
	    dbgn("Bezier.interpolate",18) << "," << cumdist[i];
	}
	dbgn("Bezier.interpolate",18) << "]" << std::endl;
	int segment=0;
	std::vector<float> told=t;
	spacing=cumdist[npoints-1]/(npoints-1);
	for (int i=0;i<npoints;i++) {
	    float desiredCumDist=spacing*i;
	    while (cumdist[segment+1]<desiredCumDist && segment<(int)cumdist.size()-1)
		segment++;
	    t[i]=told[segment]+(desiredCumDist-cumdist[segment])/(cumdist[segment+1]-cumdist[segment])*(told[segment+1]-told[segment]);
	    dbg("Bezier.interpolate",20) << "desired=" << desiredCumDist << ", seg=" << segment << ",t=" << t[i] << std::endl;
	}
	t[npoints-1]=1;
    }
    dbg("Bezier.interpolate",17) << "Final spacing = " << spacing << std::endl;
    return interpolate(t);
}

// Get length along path with given resolution
float Bezier::getLength(float res)  const {
    // Check for cached length
    if (length<0) {
	int npoints=10;
	float newlen;
	while (npoints <1000) {
	    float maxsep=1e99;
	    std::vector<Point> pts=interpolate(npoints); 
	    newlen=0;
	    for (unsigned int i=0;i<pts.size()-1;i++) {
		float dist=(pts[i+1]-pts[i]).norm();
		newlen+=dist;
		maxsep=std::min(maxsep,dist);
	    }
	    if (maxsep<res) {
		dbg("Bezier.getLength",16) << "Computed length using " << npoints << " segments for maxsep=" << maxsep << std::endl;
		break;
	    }
	    npoints*=2;
	    if (npoints>1000) {
		dbg("Bezier.getLength",1) << "Convergence failure: computed length using " << npoints << " segments for maxsep=" << maxsep << "; res=" << res << std::endl;
	    }
	}
	// Save cached value (override const)
	((Bezier *)this)->length=newlen;
    }
    return length;
}
