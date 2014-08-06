#include <iostream>
#include <cmath>
#include <set>
#include <assert.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "drawing.h"
#include "transform.h"
#include "laser.h"
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

std::vector<CPoint> Circle::getPoints(float pointSpacing, const CPoint *priorPoint) const {
    int npoints=std::ceil(getLength()/pointSpacing)+1;
    if (npoints < 5) {
	dbg("Circle.getPoints",1) << "Circle of radius " << radius << " with point spacing of " << pointSpacing << " only had " << npoints << " points; increasing to 5" << std::endl;
	npoints=5;
    }
    std::vector<CPoint> result(npoints);
    float initphase;
    if (priorPoint==0 || (center == *priorPoint))
	initphase=0;
    else {
	// Find phase closest to prior point
	Point delta = *priorPoint-center;
	initphase=atan2(delta.Y(),delta.X());
	dbg("Circle.getPoints",3) << "Delta=" << delta << ", initial phase = " << initphase << std::endl;
    }
    for (int i = 0; i < npoints; i++) {
	float phase = i * 2.0 * M_PI /(npoints-1)+initphase;
	result[i] = CPoint(cos(phase) * radius + center.X(), sin(phase) * radius + center.Y(),c);
    }
    dbg("Circle.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

float Circle::getShapeScore(const Transform &transform, const Ranges &ranges) const {
    // Same as a line perpendicular to LIDAR scan line
    Line l(center+Point(radius,0),center-Point(radius,0),Color(0,0,0));
    return l.getShapeScore(transform, ranges);
}

float Line::getShapeScore(const Transform &transform, const Ranges &ranges) const {
    float length=(p1-p2).norm();
    float score;
    if (length==0) {
	dbg("Line.getShapeScore",1) << "zero-length line" << std::endl;
	score=1e10;
    } else {
	Point p1clipped=p1;
	Point p2clipped=p2;
	transform.clipLine(p1clipped,p2clipped);
	float lengthOnScreen = (p2clipped-p1clipped).norm();
	float shadowed=ranges.fracLineShadowed(transform.getOrigin(),p1,p2);
	dbg("Line.getShapeScore",5) << "length on screen = " << lengthOnScreen << ", total Length=" << length << ", frac shadowed=" << shadowed << std::endl;
	lengthOnScreen *= (1.0f-shadowed);
	if (lengthOnScreen<length*0.99) 
	    score=lengthOnScreen/length;
	else {
	    // All on-screen, compute maximum width of line (actually physical distance of small step in laser)
	    Point devp1=transform.mapToDevice(p1);
	    Point devp2=transform.mapToDevice(p2);

	    float delta1=(p1-transform.mapToWorld(devp1+Point(0,DELTADIST))).norm();
	    float delta2=(p2-transform.mapToWorld(devp2+Point(0,DELTADIST))).norm();
	    score=1.0+1.0/std::max(delta1,delta2);
	}
    }
    dbg("Line.getShapeScore",5) << "{" <<  p1 << "; " << p2 << "} score=" << score << std::endl;
    return score;
}

std::vector<CPoint> Line::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    int npoints=std::max(2,(int)std::ceil(getLength()/pointSpacing)+1);
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
    std::vector<CPoint> result(npoints);
    for (int i = 0; i < npoints; i++) {
	float rpos=i*1.0/(npoints-1);
	if (swap) rpos=1-rpos;
	result[i] = CPoint(p1+(p2-p1)*rpos,c);
    }
    dbg("Line.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}


std::vector<CPoint> Cubic::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    std::vector<Point> pts = b.interpolate(pointSpacing);
    std::vector<CPoint> cpts(pts.size());
    for (int i=0;i<pts.size();i++) {
	cpts[i]=CPoint(pts[i],c);
	if (cpts[i].isNan()) {
	    dbg("Cubic.getPoints",1) << "Bad interpolation; pointSpacing=" << pointSpacing << ", npts=" << pts.size() << std::endl;
	    assert(0);
	}
    }
    dbg("Cubic.getPoints",3) << "Converted to " << pts.size() << " points." << std::endl;
    return cpts;
}

float Cubic::getShapeScore(const Transform &transform, const Ranges &ranges) const {
    // Approximate with fixed number of segments
    std::vector<Point> pts = b.interpolate(5);
    float score;
    float fracScore=0;
    float totalLength=0;
    for (int i=0;i<pts.size()-1;i++) {
	// Temporary line
	Line l(pts[i],pts[i+1],Color(0,0,0));
	float len=l.getLength();
	float s=l.getShapeScore(transform,ranges);
	if (s<1)
	    fracScore+=s*len;
	else
	    fracScore+=1.0*len;
	if (i==0)
	    score=s;
	else
	    score=std::min(score,s);
	totalLength+=len;
    }
    if (score<1.0 && totalLength>0)
	// Partially off-screen
	score= fracScore/totalLength;

    dbg("Cubic.getShapeScore",5) << "bezier from " << b.getPoint(0.0) << " with total length=" << totalLength << " interpolated in " << pts.size() << " points,  score=" << score << std::endl;
    return score;
}

std::vector<CPoint> Path::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    std::vector<CPoint> result;
    if (priorPoint!=NULL && (*priorPoint-controlPts.front()).norm()>.001)
	// Need to blank to get to starting position
	result.push_back(CPoint((Point)controlPts.front(),Color(0,0,0)));
    for (int i=0;i<controlPts.size()-3;i+=3) {
	std::vector<CPoint> cp=Cubic(std::vector<Point> (controlPts.begin()+i, controlPts.begin()+i+4),c).getPoints(pointSpacing,priorPoint);
	result.insert(result.end(),cp.begin(),cp.end());
	dbg("Path.getPoints",5) << "got " << cp.size() << "points from cubic, now have " << result.size() << " points.";
	for (int j=0;j<cp.size();j++) 
	    dbgn("Path.getPoints",5) << cp[j] << " ";
	dbgn("Path.getPoints",5) << std::endl;
	priorPoint=&result.back();
    }
    dbg("Path.getPoints",5) << "getPoints(spacing=" << pointSpacing << ") -> " << result.size() << " points (first=" << result.front() << ", 2nd last=" << result[result.size()-2] << ", last=" << result.back() << ")" << std::endl;
    return result;
}

float Path::getLength() const {
    float len=0;
    for (int i=0;i<controlPts.size()-3;i+=3)
	len+=Cubic(std::vector<Point> (controlPts.begin()+i, controlPts.begin()+i+4),c).getLength();
    dbg("Path.getLength",5) << "len=" << len << std::endl;
    return len;
}

float Path::getShapeScore(const Transform &transform, const Ranges &ranges) const {
    dbg("Path.getShapeScore",5) << "Getting score for path" << std::endl;
    float score;
    float visLen=0;
    float totalLen=0;
    for (int i=0;i<controlPts.size()-3;i+=3) {
	float s=Cubic(std::vector<Point> (controlPts.begin()+i,controlPts.begin()+i+4),c).getShapeScore(transform,ranges);
	float len=Cubic(std::vector<Point> (controlPts.begin()+i, controlPts.begin()+i+4),c).getLength();
	if (i==0)
	    score=s;
	else
	    score=std::min(score,s);
	visLen+=std::min(1.0f,s)*len;
	totalLen+=len;
    }
    if (score<1.0 && totalLen>0)
	score=visLen/totalLen;
    dbg("Path.getShapeScore",5) << "score=" << score << std::endl;
    return score;
}

std::vector<CPoint> Polygon::getPoints(float pointSpacing,const CPoint *priorPoint) const {
    std::vector<CPoint> result;
    for (int i=1;i<points.size();i++) {
	std::vector<CPoint> line=Line(points[i-1],points[i],c).getPoints(pointSpacing,priorPoint);
	result.insert(result.end(),line.begin(),line.end());
	priorPoint=&result.back();
    }
    dbg("Polygon.getPoints",5) << "getPoints(spacing=" << pointSpacing << ") -> " << result.size() << " points" << std::endl;
    return result;
}

float Polygon::getLength() const {
    if (points.size()<=1)
	return 0.0f;
    Point lastpt=points.back();
    float len=0;
    for (int i=0;i<points.size();i++) {
	len+=(lastpt-points[i]).norm();
	lastpt=points[i];
    }
    return len;
}

std::vector<CPoint> Arc::getPoints(float pointSpacing, const CPoint *priorPoint) const {
    assert(0);   // TODO
}


float Composite::getShapeScore(const Transform &transform, const Ranges &ranges) const {
    dbg("Composite.getShapeScore",5) << "Getting score for composite with " << elements.size() << " elements"  << std::endl;
    float score;
    float fracScore=0;
    float totalLen=0;
    for (unsigned int i=0;i<elements.size();i++) {
	float s=elements[i]->getShapeScore(transform,ranges);
	float len=elements[i]->getLength();
	if (i==0)
	    score=s;
	else
	    score=std::min(score,s);
	fracScore+=std::min(1.0f,s)*len;
	totalLen+=len;
    }
    if (score<1.0 && totalLen>0)
	score=fracScore/totalLen;
    dbg("Composite.getShapeScore",5) << "score=" << score << std::endl;
    return score;
}

// Get quality score of reproduction for each of the elements within the drawing using the given transform
std::map<int,float> Drawing::getShapeScores(const Transform &transform, const Ranges &ranges) const {
    std::map<int,float> scores;
    for (unsigned int i=0;i<elements.size();i++)
	scores[i]=elements[i]->getShapeScore(transform,ranges);
    return scores;
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
std::vector<CPoint> Composite::getPoints(float spacing,const CPoint *priorPoint) const {
    dbg("Composite.getPoints",3) << "getPoints(" << spacing << ") with " << attrs.size() << " attributes"  << std::endl;
    std::vector<CPoint>  result;
    if (elements.size()==0) {
	dbg("Composite.getPoints",3) << "No subelements" << std::endl;
	return result;
    }

    for (unsigned int i=0;i<elements.size();i++) {
	std::vector<CPoint> newpoints;
	newpoints = elements[i]->getPoints(spacing,priorPoint);

	if (priorPoint!=NULL && newpoints.size()>0 && !(newpoints.front() == *priorPoint))  {
	    // Insert a blank to make the jump
	    result.push_back(CPoint((Point)newpoints.front(),Color(0,0,0)));
	}
	result.insert(result.end(), newpoints.begin(), newpoints.end());
	if (result.size()>0)
	    priorPoint=&result.back();
    }
    dbg("Composite.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    if (drawConvexHull) {
	dbg("Composite.getPoints",3) << "Converting into a convex hull" << std::endl;
	std::vector<cv::Point2f> src(result.size());
	for (int i=0;i<result.size();i++) {
	    src[i].x=result[i].X();
	    src[i].y=result[i].Y();
	}
	std::vector<cv::Point2f> dst;
	cv::convexHull(src,dst);
	dbg("Composite.getPoints",3) << "Converted " << result.size() << " points into a convex hull of size " << dst.size() << std::endl;
	result.resize(dst.size());
	for (int i=0;i<dst.size();i++) {
	    result[i]=CPoint(dst[i].x,dst[i].y,elements.front()->getColor());
	}
	result=CPoint::resample(result);
	// TODO -- hull might have larger spacing between some points
    }

    // Apply all the attributes to the points
    result=attrs.apply(result);
    return result;
}

// Convert to points using given floorspace spacing
std::vector<CPoint> Drawing::getPoints(float spacing) const {
    dbg("Drawing.getPoints",3) << "getPoints(" << spacing << ")" << std::endl;
    std::vector<CPoint>  result;
    if (elements.size()==0)
	return result;

    for (unsigned int i=0;i<elements.size();i++) {
	std::vector<CPoint> pts;
	if (result.size()==0) {
	    pts= elements[i]->getPoints(spacing,NULL);
	    result.insert(result.end(), pts.begin(), pts.end());
	} else {
	    pts= elements[i]->getPoints(spacing,&result.back());
	    if (pts.size()>0) {
		if (!(pts.front()==result.back()))
		    // Insert blanks first
		    result.push_back(CPoint((Point)pts.front(),Color(0,0,0)));
		result.insert(result.end(), pts.begin(), pts.end());
	    }
	}
    }
    if (result.size()>1 && !(result.front()==result.back()))
	// One more blank at end
	result.push_back(CPoint((Point)result.front(),Color(0,0,0)));
    dbg("Drawing.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

std::vector<CPoint> Drawing::clipPoints(const std::vector<CPoint> &pts, const Bounds &b) const {
    std::vector<CPoint> result;
    bool wasBlank=false;
    for (int i=0;i<pts.size();i++) {
	bool isBlank=(pts[i].getColor() == Color(0,0,0)) || !b.contains(pts[i]);
	if (wasBlank && !isBlank) {
	    result.push_back(CPoint(pts[i],Color(0,0,0)));  // Move to new point with blanking
	}
	if (!isBlank)
	    result.push_back(pts[i]);
	//	dbg("Drawing.clipPoints",4) << "i=" << i << ", wasBlank=" << wasBlank << ", isBlank=" << isBlank << ", result.size=" << result.size() << std::endl;
	wasBlank=isBlank;
    }
    if (wasBlank)
	// Make sure we get the final blank
	result.push_back(CPoint(pts.back(),Color(0,0,0)));
    dbg("Drawing.clipPoints",3) << "Clipped drawing from " << pts.size() << " to " << result.size() << " points." << std::endl;
    return result;
}
