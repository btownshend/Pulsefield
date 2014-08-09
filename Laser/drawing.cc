#include <iostream>
#include <cmath>
#include <set>
#include <assert.h>
#include "drawing.h"
#include "transform.h"
#include "dbg.h"

const float Primitive::DELTADIST=20;  // Distance to move in device space to calculate spread do to angle
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

CPoints Circle::getPoints(float pointSpacing, const CPoint *priorPoint) const {
    int npoints=std::ceil(2*M_PI*radius/pointSpacing)+1;
    if (npoints < 5) {
	dbg("Circle.getPoints",1) << "Circle of radius " << radius << " with point spacing of " << pointSpacing << " only had " << npoints << " points; increasing to 5" << std::endl;
	npoints=5;
    }
    CPoints result;
    float initphase;
    if (priorPoint==0 || (center == *priorPoint))
	initphase=0;
    else {
	// Find phase closest to prior point
	Point delta = *priorPoint-center;
	initphase=atan2(delta.Y(),delta.X());
	dbg("Circle.getPoints",5) << "Delta=" << delta << ", initial phase = " << initphase << std::endl;
    }
    for (int i = 0; i < npoints; i++) {
	float phase = i * 2.0 * M_PI /(npoints-1)+initphase;
	result.push_back(CPoint(cos(phase) * radius + center.X(), sin(phase) * radius + center.Y(),(i==0)?Color(0,0,0):c));
    }
    dbg("Circle.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

CPoints Line::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    int npoints=std::max(2,(int)std::ceil((p2-p1).norm()/pointSpacing)+1);
    bool swap=false;
    if (priorPoint!=NULL) {
	// Check if line direction should be swapped
	int d1=std::max(fabs(priorPoint->X()-p1.X()),fabs(priorPoint->Y()-p1.Y()));
	int d2=std::max(fabs(priorPoint->X()-p2.X()),fabs(priorPoint->Y()-p2.Y()));
	if (d2<d1) {
	    dbg("Line.getPoints",3) << "Swapping line endpoints; d1=" << d1 << ", d2=" << d2 << std::endl;
	    swap=true;
	}
    }
    CPoints result;
    for (int i = 0; i < npoints; i++) {
	float rpos=i*1.0/(npoints-1);
	if (swap) rpos=1-rpos;
	result.push_back(CPoint(p1+(p2-p1)*rpos,(i==0)?Color(0,0,0):c));
    }
    dbg("Line.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}


CPoints Cubic::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    std::vector<Point> pts = b.interpolate(pointSpacing);
    CPoints cpts;
    for (int i=0;i<pts.size();i++) {
	CPoint cpt(pts[i],(i==0)?Color(0,0,0):c);
	if (cpt.isNan()) {
	    dbg("Cubic.getPoints",1) << "Bad interpolation; pointSpacing=" << pointSpacing << ", npts=" << pts.size() << std::endl;
	    assert(0);
	}
	cpts.push_back(cpt);
    }
    // Resample to make spacing uniform (since bezier interpolate isn't uniform)
    cpts=cpts.resample(pointSpacing);
    dbg("Cubic.getPoints",3) << "Converted to " << pts.size() << " points." << std::endl;
    return cpts;
}

CPoints Path::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    CPoints result;
    for (int i=0;i<controlPts.size()-3;i+=3) {
	Bezier b(std::vector<Point> (controlPts.begin()+i, controlPts.begin()+i+4));
	std::vector<Point> bpts=b.interpolate(pointSpacing);
	assert(bpts.size()>=2);
	if (i==0)
	    // Only need the first point from the first bezier (as a move), the others are redundant
	    result.push_back(CPoint(bpts[0],Color(0,0,0)));
	for (int j=1;j<bpts.size();j++)
	    result.push_back(CPoint(bpts[j],c));
	dbg("Path.getPoints",5) << "got " << bpts.size() << "points from bezier, now have " << result.size() << " points.";
	priorPoint=&result.back();
    }
    // Change first entry into a blank (moveto)
    result.front().setColor(Color(0,0,0));
    // Resample to make spacing uniform (in case of many small paths)
    result=result.resample(pointSpacing);
    dbg("Path.getPoints",5) << "getPoints(spacing=" << pointSpacing << ") -> " << result.size() << " points" << std::endl;
    return result;
}

CPoints Polygon::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    CPoints result;
    for (int i=1;i<points.size();i++) {
	CPoints line=Line(points[i-1],points[i],c).getPoints(pointSpacing,priorPoint);
	if (i>1)
	    // Change first entry of each line back to a lineto
	    line[0].setColor(c);
	result.append(line);
	priorPoint=&result.back();
    }
    dbg("Polygon.getPoints",5) << "getPoints(spacing=" << pointSpacing << ") -> " << result.size() << " points" << std::endl;
    return result;
}

CPoints Arc::getPoints(float pointSpacing, const CPoint *priorPoint) const {
    assert(0);   // TODO
}

// Get only the drawing elements in the given set of elements
Drawing Drawing::select(std::set<int> sel) const {
    Drawing result;
    result.setFrame(frame);
    for (unsigned int i=0;i<elements.size();i++) {
	if (sel.count(i) > 0)
	    result.elements.push_back(elements[i]);
    }
    dbg("Drawing.select",4) << "Selected " << result.getNumElements() << "/" << getNumElements()  << std::endl;
    assert(result.getNumElements()==sel.size());
    return result;
}

// Convert to points using given floorspace spacing
void Composite::rasterize(float spacing)  {
    dbg("Composite.rasterize",3) << "rasterize(" << spacing << ") with " << attrs.size() << " attributes"  << std::endl;
    if (elements.size()==0) {
	dbg("Composite.rasterize",3) << "No subelements" << std::endl;
	return;
    }

    const CPoint *priorPoint=NULL;
    for (unsigned int i=0;i<elements.size();i++) {
	CPoints newpoints;
	newpoints = elements[i]->getPoints(spacing,priorPoint);

	// First point of any object should be a moveto
	assert(newpoints.size()==0 || newpoints.front().getColor()==Color(0,0,0));
	points.append(newpoints);
	if (points.size()>0)
	    priorPoint=&points.back();
    }
    dbg("Composite.rasterize",3) << "Converted to " << points.size() << " points." << std::endl;
    if (drawConvexHull) {
	points=points.convexHull(spacing,elements[0]->getColor());	// Draw with same color as first element had
	dbg("Composite.rasterize",3) << "Converted to convex hull with " << points.size() << " points." << std::endl;
    }

    // Apply all the attributes to the points
    points=attrs.apply(points);
    rasterSpacing=spacing;
}

// Convert to points using given floorspace spacing
CPoints Drawing::getPoints(float spacing) const {
    dbg("Drawing.getPoints",3) << "getPoints(" << spacing << ")" << std::endl;
    CPoints  result;
    if (elements.size()==0)
	return result;

    for (unsigned int i=0;i<elements.size();i++) {
	CPoints pts;
	if (elements[i]->getRasterSpacing()==0)
	    elements[i]->rasterize(spacing);
	if (std::abs(spacing-elements[i]->getRasterSpacing()) < 0.001) {
	    // No need to resample
	    pts=elements[i]->getPoints();
	}  else {
	    pts=elements[i]->getPoints().resample(spacing);
	}
	result.append(pts);
    }
    dbg("Drawing.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

