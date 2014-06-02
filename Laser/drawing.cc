#include <iostream>
#include <cmath>
#include <set>
#include <assert.h>
#include "opencv2/imgproc/imgproc.hpp"
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

std::vector<Point> Circle::getPoints(float pointSpacing, const Point *priorPoint) const {
    int npoints=std::ceil(getLength()/pointSpacing)+1;
    if (npoints < 5) {
	dbg("Circle.getPoints",1) << "Circle of radius " << radius << " with point spacing of " << pointSpacing << " only had " << npoints << " points; increasing to 5" << std::endl;
	npoints=5;
    }
    std::vector<Point> result(npoints);
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
	result[i] = Point(cos(phase) * radius + center.X(), sin(phase) * radius + center.Y());
    }
    dbg("Circle.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}

float Circle::getShapeScore(const Transform &transform) const {
    Point devcenter=transform.mapToDevice(center);
    float devradius=(transform.mapToDevice(Point(center.X(),center.Y()+radius))-devcenter).norm();
    float score;
    if (devcenter.X()+devradius>32767 || devcenter.X()-devradius<-32768 || devcenter.Y()+devradius>32767 || devcenter.Y()-devradius<-32768)
	score=0.0;  // off-screen
    else {
	// All on-screen, compute maximum width of line (actually physical distance of small step in laser)
	float delta1=(center-transform.mapToWorld(devcenter+Point(0,1))).norm();
	float delta2=(center-transform.mapToWorld(devcenter+Point(1,0))).norm();
	score=1.0/std::hypot(delta1,delta2)+1.0;
    }
    dbg("Circle.getShapeScore",5) <<  center << " maps to " << devcenter << " score=" << score << std::endl;
    return score;
}

float Line::getShapeScore(const Transform &transform) const {
    Point devp1=transform.mapToDevice(p1);
    Point devp2=transform.mapToDevice(p2);
    float length=(devp1-devp2).norm();
    float score;
    if (length==0) {
	dbg("Line.getShapeScore",1) << "zero-length line" << std::endl;
	score=1e10;
    } else {
	float lengthOnScreen = (devp1.min(Point(32767,32767)).max(Point(-32768,-32768))-devp2.min(Point(32767,32767)).max(Point(-32768,-32768))).norm();
	if (lengthOnScreen<length*0.99) 
	    score=lengthOnScreen/length;
	else {
	    // All on-screen, compute maximum width of line (actually physical distance of small step in laser)
	    Point dir=devp2-devp1; dir=dir/dir.norm();
	    Point orthogonal(-dir.Y(),dir.X());
	    float delta1=(p1-transform.mapToWorld(devp1+orthogonal)).norm();
	    float delta2=(p2-transform.mapToWorld(devp2+orthogonal)).norm();
	    score=1.0+1.0/std::max(delta1,delta2);
	}
    }
    dbg("Line.getShapeScore",5) << "{" <<  p1 << "; " << p2 << "} maps to {" << devp1 << "; " << devp2 << "} score=" << score << std::endl;
    return score;
}

std::vector<Point> Line::getPoints(float pointSpacing,const Point *priorPoint) const {
    int npoints=std::max(2,(int)std::ceil(getLength()/pointSpacing)+1);
    bool swap=false;
    if (priorPoint!=NULL) {
	// Check if line direction should be swapped
	int d1=std::max(abs(priorPoint->X()-p1.X()),abs(priorPoint->Y()-p1.Y()));
	int d2=std::max(abs(priorPoint->X()-p2.X()),abs(priorPoint->Y()-p2.Y()));
	if (d2<d1) {
	    dbg("Line.getPoints",3) << "Swapping line endpoints; d1=" << d1 << ", d2=" << d2 << std::endl;
	    swap=true;
	}
    }
    std::vector<Point> result(npoints);
    for (int i = 0; i < npoints; i++) {
	float rpos=i*1.0/(npoints-1);
	if (swap) rpos=1-rpos;
	result[i] = p1+(p2-p1)*rpos;
    }
    dbg("Line.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
    return result;
}


std::vector<Point> Cubic::getPoints(float pointSpacing,const Point *priorPoint) const {
    std::vector<Point> pts = b.interpolate(pointSpacing);
    dbg("Cubic.getPoints",3) << "Converted to " << pts.size() << " points." << std::endl;
    return pts;
}

float Cubic::getShapeScore(const Transform &transform) const {
    // Approximate with fixed number of segments
    std::vector<Point> pts = b.interpolate(5);
    float score;
    float fracScore=0;
    for (int i=0;i<pts.size()-1;i++) {
	// Temporary line
	Line l(pts[i],pts[i+1],Color(0,0,0));
	float s=l.getShapeScore(transform);
	if (s<1)
	    fracScore+=s;
	else
	    fracScore+=1.0;
	if (i==0)
	    score=s;
	else
	    score=std::min(score,s);
    }
    if (score<1)
	// Partially off-screen
	score= fracScore/pts.size();

    dbg("Cubic.getShapeScore",5) << "score=" << score << std::endl;
    return score;
}

std::vector<Point> Polygon::getPoints(float pointSpacing,const Point *priorPoint) const {
    std::vector<Point> result;
    for (int i=1;i<points.size();i++) {
	std::vector<Point> line=Line(points[i-1],points[i],c).getPoints(pointSpacing,priorPoint);
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

std::vector<etherdream_point> Primitive::convert(const std::vector<Point> &pts, const Transform &transform) const {
    std::vector<etherdream_point> result(pts.size());
    for (unsigned int i = 0; i < pts.size(); i++) {
	if (i>0 && pts[i]==pts[i-1]) {
	    // Blanking, turn off laser
	    result[i-1].r=0;result[i-1].g=0;result[i-1].b=0;  
	    result[i]=result[i-1]; 
	} else
	    result[i] = transform.mapToDevice(pts[i],c);
    }
    return result;
}

std::vector<Point> Arc::getPoints(float pointSpacing, const Point *priorPoint) const {
    assert(0);   // TODO
}


float Composite::getShapeScore(const Transform &transform) const {
    dbg("Composite.getShapeScore",5) << "Getting score for composite" << std::endl;
    float score;
    float fracScore=0;
    for (unsigned int i=0;i<elements.size();i++) {
	float s=elements[i]->getShapeScore(transform);
	if (i==0)
	    score=s;
	else
	    score=std::min(score,s);
	fracScore+=std::min(1.0f,s);
    }
    if (score<1.0)
	score=fracScore/elements.size();
    dbg("Composite.getShapeScore",5) << "score=" << score << std::endl;
    return score;
}

// Get quality score of reproduction for each of the elements within the drawing using the given transform
std::map<int,float> Drawing::getShapeScores(const Transform &transform) const {
    std::map<int,float> scores;
    for (unsigned int i=0;i<elements.size();i++)
	scores[i]=elements[i]->getShapeScore(transform);
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
    dbg("Drawing.select",2) << "Selected " << result.getNumElements() << "/" << getNumElements()  << std::endl;
    assert(result.getNumElements()==sel.size());
    return result;
}

// Convert to points using given floorspace spacing
std::vector<Point> Composite::getPoints(float spacing,const Point *priorPoint) const {
    dbg("Composite.getPoints",3) << "getPoints(" << spacing << ") with " << attrs.size() << " attributes"  << std::endl;
    std::vector<Point>  result;
    if (elements.size()==0) {
	dbg("Composite.getPoints",3) << "No subelements" << std::endl;
	return result;
    }

    for (unsigned int i=0;i<elements.size();i++) {
	std::vector<Point> newpoints;
	newpoints = elements[i]->getPoints(spacing,priorPoint);

	if (priorPoint!=NULL && newpoints.size()>0)  {
	    // Insert blanks first
	    std::vector<Point> blanks = Laser::getBlanks(*priorPoint,newpoints.front());
	    result.insert(result.end(), blanks.begin(), blanks.end());
	    dbg("Composite.getPoints",3) << "Inserted " << blanks.size() << " blanks before element " << i << " which has " << newpoints.size() << " elements" << std::endl;
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
	    result[i]=Point(dst[i].x,dst[i].y);
	}
	// TODO -- hull might have larger spacing between some points
    }

    // Apply all the attributes to the points
    result=attrs.apply(result);
    return result;
}


// Convert to points using given floorspace spacing
std::vector<etherdream_point> Drawing::getPoints(float spacing,const Transform &transform) const {
    dbg("Drawing.getPoints",3) << "getPoints(" << spacing << ")" << std::endl;
    std::vector<etherdream_point>  result;
    if (elements.size()==0)
	return result;

    const Point *lastPoint = NULL;
    for (unsigned int i=0;i<elements.size();i++) {
	std::vector<Point> pts = elements[i]->getPoints(spacing,lastPoint);
	if (pts.size() > 0)
	    lastPoint = &pts.back();
	std::vector<etherdream_point> newpoints = elements[i]->convert(pts,transform);

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
    dbg("Drawing.getPoints",3) << "Converted to " << result.size() << " points." << std::endl;
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
    if (elements.size()==0)
	return std::vector<etherdream_point>();

    spacing=getLength()/targetNumPoints;
    std::vector<etherdream_point> result = getPoints(spacing,transform);
    std::vector<etherdream_point> pruned = prune(result);
    dbg("Drawing.getPoints",2) << "Initial point count = " << pruned.size() << " compared to planned " << targetNumPoints << " for " << elements.size() << " elements." << std::endl;
    int nblanks = 0;
    for (unsigned int i=0;i<pruned.size();i++)
	if (pruned[i].r==0 && pruned[i].g==0 && pruned[i].b==0)
	    nblanks++;
    if (nblanks> targetNumPoints/2)  {
	targetNumPoints=nblanks*2;
	dbg("Drawing.getPoints",2) << "More than half of points are blanks, increasing target to " << targetNumPoints << std::endl;
    }

    if (std::abs(targetNumPoints-(int)pruned.size()) > 10 &&  pruned.size() > nblanks+2) {
	float scaleFactor=(targetNumPoints-nblanks)*1.0/(pruned.size()-nblanks);
	dbg("Drawing.getPoints",2) << "Have " << nblanks << " blanks; adjusting spacing by a factor of " << scaleFactor << std::endl;
	spacing/=scaleFactor;
	result = getPoints(spacing,transform);
	pruned = prune(result);
	dbg("Drawing.getPoints",2) << "Revised point count = " << pruned.size() << " compared to planned " << targetNumPoints << " for " << elements.size() << " elements." << std::endl;
    }
    return pruned;
}

