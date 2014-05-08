#pragma once 
#include <ostream>
#include <vector>
#include "point.h"
#include "etherdream.h"
#include "laser.h"
#include "dbg.h"
#include "bezier.h"
#include "opencv2/core/core.hpp"

class Color {
    float r,g,b;
 public:
    // New color with give RGB (0.0 - 1.0)
    Color(float _r, float _g, float _b) { r=_r; g=_g; b=_b; }
    float red() const { return r; }
    float green() const { return g; }
    float blue() const { return b; }
    friend std::ostream& operator<<(std::ostream& s, const Color &col);
};

// Transformation between floor (meter) space and device (-32767:32767) space
class Transform {
    cv::Mat transform; 
    std::vector<cv::Point2f> floorpts, devpts;
 public:
    Transform();
    void clear();
    // Set transform based on floor space coordinates of particular device space points
    void set(std::vector<Point> devPts, std::vector<Point> floorPts);
    // Mapping, if out-of-range, return clipped point
    etherdream_point mapToDevice(Point floorPt,Color c) const;
    std::vector<etherdream_point> mapToDevice(std::vector<Point> floorPts,Color c) const;

    // Computer transform matrix from set of points already provided
    void setTransform();
    void addToMap(Point devSpace, Point floorSpace) {
	floorpts.push_back(cv::Point2f(floorSpace.X(), floorSpace.Y()));
	devpts.push_back(cv::Point2f(devSpace.X(), devSpace.Y()));
    }
};

class Primitive {
protected:
    Color c;
 public:
    Primitive(Color _c): c(_c) {;}
    virtual ~Primitive() { ; }
    // Get list of discrete points spaced approximately by pointSpacing,  optimizes based on minimizing distance from priorPoint to first point
    // Converts from floor space to device coordinates in the process
    virtual std::vector<etherdream_point> getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const {
	return std::vector<etherdream_point>(0);
    }
    virtual float getLength() const { return 0.0; }
};

class Circle: public Primitive {
    Point center;
    float radius;
 public:
    Circle(Point _c, float r, Color c): Primitive(c) {
	dbg("Circle",2) << "c=" <<_c << ", r=" << r << std::endl;
	center=_c; radius=r;
    }
    std::vector<etherdream_point> getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const;
    float getLength() const { return radius*2*M_PI; }
};

class Arc: public Primitive {
    Point center, p;
    float angle;
 public:
    Arc(Point _center, Point _p, float _angle, Color c): Primitive(c) { center=_center; p=_p; angle=_angle; }
    // TODO
};

class Line:public Primitive {
    Point p1,p2;
 public:
    Line(Point _p1, Point _p2, Color c): Primitive(c) { p1=_p1; p2=_p2;  }
    std::vector<etherdream_point> getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const;
    float getLength() const { return (p1-p2).norm(); }
};

class Cubic:public Primitive {
    Bezier b;
 public:
    Cubic(const std::vector<Point> &pts, Color c): Primitive(c), b(pts) {;}
    std::vector<etherdream_point> getPoints(float pointSpacing,const Transform &transform,const etherdream_point *priorPoint) const;
    float getLength() const { return b.getLength(); }
};

class Drawing {
    std::vector<Primitive *> elements;
    Transform transform;	// 3x3 transformation matrix; maps floor position in meters to device coords
 public:
    Drawing() { ; }

    // Set transform matrix
    void setTransform() {
	transform.setTransform();
    }

    void addToMap(Point devSpace, Point floorSpace) {
	transform.addToMap(devSpace,floorSpace);
    }

    // Number of primitives (which gives number of jumps)
    int getNumElements() const { return elements.size(); }

    // Get length of current drawing in floor space
    float getLength() const {
	float len=0;
	for (unsigned int i=0;i<elements.size();i++) {
	    len+=elements[i]->getLength();
	}
	dbg("Drawing.getLength",3) << "length=" << len << " for " << elements.size() << " elements." << std::endl;
	return len;
    }

    // Clear drawing
    void clear() {
	for (unsigned int i=0;i<elements.size();i++)
	    delete elements[i];
	elements.clear();  
    }

    // Add a circle to current drawing
    void drawCircle(Point center, float r, Color c) {
	dbg("Drawing.drawCircle",2) << "center=" << center << ", radius=" << r << ", color=" << c << std::endl;
	elements.push_back(new Circle(center,r,c));
    }

    // Add a arc to current drawing
    void drawArc(Point center, Point p, float angle, Color c) {
	dbg("Drawing.drawArc",2) << "center=" << center << ", Point =" << p << ", angleCW=" << angle << ", color=" << c << std::endl;
	elements.push_back(new Arc(center,p,angle,c));
    }

    // Add a line to current drawing
    void drawLine(Point p1, Point p2, Color c) {
	elements.push_back(new Line(p1,p2,c));
    }

    // Add a cubic to current drawing
    void drawCubic(Point p1, Point p2, Point p3, Point p4, Color c) {
	std::vector<Point> pts;
	pts[0]=p1;pts[1]=p2;pts[2]=p3;pts[3]=p4;
	elements.push_back(new Cubic(pts,c));
    }


    // Convert drawing into a set of etherdream points
    // Takes into account transformation to make all lines uniform brightness (i.e. separation of points is constant in floor dimensions)
    std::vector<etherdream_point> getPoints(int targetNumPoints) const;

    // Convert to points using given floorspace spacing
    std::vector<etherdream_point> getPoints(float spacing) const;
};
