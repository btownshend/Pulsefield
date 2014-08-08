#pragma once

#include "cpoint.h"

// Class to deal with a rectangular bounds
class Bounds {
    float minx, miny, maxx, maxy;
 public:
    Bounds(float minx, float miny, float maxx, float maxy) {
	this->minx=minx; this->miny=miny;this->maxx=maxx;this->maxy=maxy;
    }
    float getMinX() const { return minx; }
    float getMinY() const { return miny; }
    float getMaxX() const { return maxx; }
    float getMaxY() const { return maxy; }

    void setMinX(float minx) { this->minx=minx; }
    void setMaxX(float maxx) { this->maxx=maxx; }
    void setMinY(float miny) { this->miny=miny; }
    void setMaxY(float maxy) { this->maxy=maxy; }

    // Constrain a point to be within bounds
    Point constrainPoint(Point p) const{ 
	if (p.isNan())
	    return p;
	return Point(std::min(maxx,std::max(minx,p.X())),std::min(maxy,std::max(miny,p.Y())));
    }
    // Constrain a point to be within bounds
    CPoint constrainPoint(CPoint p) const{ 
	if (p.isNan())
	    return p;
	return CPoint(std::min(maxx,std::max(minx,p.X())),std::min(maxy,std::max(miny,p.Y())),p.getColor());
    }
    // Return true of point p is in bounds
    bool contains(Point p) const {
	return  isfinite(p.X()) & isfinite(p.Y()) && (p.X() >= minx) && (p.X() <=maxx) && (p.Y() >= miny) && (p.Y() <= maxy);
    }
    float width() const { return maxx-minx; }
    float height() const { return maxy-miny; }
    friend std::ostream& operator<<(std::ostream &s, const Bounds &b);
    bool operator==(const Bounds &b) const { return b.minx==minx && b.miny==miny && b.maxx==maxx && b.maxy==maxy; }
};

inline std::ostream& operator<<(std::ostream &s, const Bounds &b) {
    return s << "[" << b.minx << "," << b.miny << "," << b.maxx << "," << b.maxy << "]";
}



