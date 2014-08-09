#pragma once 
#include <ostream>
#include <vector>
#include <map>
#include <set>

#include "point.h"
#include "dbg.h"
#include "bezier.h"
#include "color.h"
#include "attributes.h"
#include "cpoint.h"
#include "ranges.h"
#include "bounds.h"
#include "shapeid.h"

class Transform;
class Drawing;

class Primitive {
protected:
    static const float DELTADIST;  // Distance to move in device space to calculate spread do to angle
    Color c;
 public:
    Primitive(Color _c): c(_c) { }
    virtual ~Primitive() { ; }
    // Get list of discrete points spaced approximately by pointSpacing,  optimizes based on minimizing distance from priorPoint to first point
    virtual CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const = 0;

    virtual void translate(Point t) = 0; 

    Color getColor() const { return c; }
};

class Circle: public Primitive {
    Point center;
    float radius;
 public:
    Circle(Point _c, float r, Color c): Primitive(c) {
	dbg("Circle",2) << "c=" <<_c << ", r=" << r << std::endl;
	center=_c; radius=r;
    }
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { center=center+t; }
};

class Arc: public Primitive {
    Point center, p;
    float angle;
 public:
    Arc(Point _center, Point _p, float _angle, Color c): Primitive(c) { center=_center; p=_p; angle=_angle; }
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { center=center+t; p=p+t; }
};

class Line:public Primitive {
    Point p1,p2;
 public:
    Line(Point _p1, Point _p2, Color c): Primitive(c) { p1=_p1; p2=_p2;  }
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { p1=p1+t; p2=p2+t; }
};

class Cubic:public Primitive {
    Bezier b;
 public:
    Cubic(const std::vector<Point> &pts, Color c): Primitive(c), b(pts) {;}
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { b.translate(t); }
};

// A path formed by a set of beziers
class Path:public Primitive {
    std::vector<Point> controlPts;
 public:
    Path(const std::vector<Point> &p, Color c): Primitive(c), controlPts(p) {;}
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { 
	for (int i=0;i<controlPts.size();i++)
	    controlPts[i]=controlPts[i]+t;
    }
};


class Polygon: public Primitive {
    std::vector<Point> points;
 public:
    Polygon(const std::vector<Point> _points,Color c): Primitive(c), points(_points) {;}
    CPoints getPoints(float pointSpacing,const CPoint *priorPoint) const;
    void translate(Point t) { 
	for (int i=0;i<points.size();i++)
	    points[i]=points[i]+t;
    }
};

class Composite  {
    std::vector<std::shared_ptr<Primitive> > elements;
    Attributes attrs;
    bool drawConvexHull;   // Draw convex hull rather than actual points
    std::shared_ptr<ShapeID> shapeID;
    CPoints points;  // Result of converting all elements to points at a fixed spacing
    float rasterSpacing;   // Spacing of points when rasterized, or 0 if not rasterized
public:
    Composite(const std::string &id, const Attributes _attrs, bool _hull=false) {shapeID=ShapeIDs::get(id); attrs=_attrs; drawConvexHull=_hull; }
    // Number of primitives
    int getNumElements() const { return elements.size(); }

    // Shape ID
    std::shared_ptr<ShapeID> getShapeID() const { return shapeID; }
    void append(std::shared_ptr<Primitive> p) { elements.push_back(p);clearRaster(); }

    // Convert into a vector of points (actually segments), inserts a single blank point for moves
    void rasterize(float pointSpacing);
    const CPoints &getPoints() const { return points; }
    // Reset points, assumes rasterSpacing is unchanged
    void setPoints(const CPoints &cpts) { points=cpts; }
    void setAttributes(const Attributes &a) { attrs=a; }
    // Get attributes, allowing them to be modified
    Attributes &getAttributes() { return attrs; }

    float getLength() const { return points.getLength(); }

    void translate(Point t) { 
	//	assert(points.size()==0);	// Only before rasterizing
	for (int i=0;i<elements.size();i++)
	    elements[i]->translate(t);
	clearRaster();
    }
    void clearRaster() {
	points=CPoints();
	rasterSpacing=0;
    }
    float getRasterSpacing() const { return rasterSpacing; }

    // Clip a vector of points by removing segments out of bounds and inserting blanks
    void  clip( const Bounds &b) {
	points=points.clip(b);
    }
};


class Drawing {
    std::vector<std::shared_ptr<Composite> > elements;
    int frame;  // Frame number that this drawing corresponds to (or -1 if unknown)
    bool inComposite;
 public:
    Drawing() { frame=-1; inComposite=false; }

    void setFrame(int _frame) { frame=_frame; }
    int getFrame() const { return frame; }

    // Number of primitives (which gives number of jumps)
    int getNumElements() const { return elements.size(); }
    std::shared_ptr<Composite> &getElement(int i) { return elements[i]; }

    // Get points to render drawing at given (meter) spacing
    CPoints getPoints(float spacing) const;

    // Rasterize all the subelements
    void rasterize(float pointSpacing) { 
	for (int i=0;i<getNumElements();i++)
	    getElement(i)->rasterize(pointSpacing); 
    }

    // Clip all the subelements
    void clip(const Bounds &b) { 
	for (int i=0;i<getNumElements();i++)
	    getElement(i)->clip(b); 
    }

    // Get length of current drawing in floor space
    float getLength() const {
	float len=0;
	for (unsigned int i=0;i<elements.size();i++) {
	    len+=elements[i]->getLength();
	}
	dbg("Drawing.getLength",3) << "length=" << len << " for " << elements.size() << " elements." << std::endl;
	return len;
    }
    
    void translate(Point t) {
	for (unsigned int i=0;i<elements.size();i++)
	    elements[i]->translate(t);
    }

    // Clear drawing
    void clear() {
	dbg("Drawing.clear",5) << "Clearing " << elements.size() << " elements from frame " << frame << std::endl;
	elements.clear();  
	inComposite=false;
	frame=-1;
    }

    // Get a subset of the drawing containing only the given elements
    Drawing select(std::set<int> elements) const ;

    bool isShapeOpen() const { return inComposite; };

    void append(std::shared_ptr<Primitive> prim) {
	if (inComposite) {
	    // Append to current composite 
	    if (elements.size()==0) {
		dbg("Drawing.append",0) << "inComposite but no elements" << std::endl;
		inComposite=false;
	    } else {
		std::shared_ptr<Composite> c=std::dynamic_pointer_cast<Composite>(elements.back());
		if (c==std::shared_ptr<Composite>()) {
		    dbg("Drawing.append",0) << "Last element is not a composite" << std::endl;
		    inComposite=false;
		} else {
		    c->append(prim);
		    return;
		}
	    }
	}
	dbg("Drawing.append",0) << "Attempt to append an element when not in a composite" << std::endl;
    }

    // Start a new shape
    void shapeBegin(const std::string &id, const Attributes &attr, bool drawConvexHull=false) {
	dbg("Drawing.shapeBegin",3) << "begin shape " << id << " with " << attr.size() << " attributes" << std::endl;
	if (inComposite) {
	    std::string lastShapeID=elements.back()->getShapeID()->getID() ;
	    // OK if the current composite is empty
	    if (elements.back()->getNumElements() == 0) {
		dbg("Drawing.shapeBegin",1) << "Was already in empty composite " << lastShapeID <<  " when beginning " << id << std::endl;
	    } else {
		dbg("Drawing.shapeBegin",0) << "Was already in composite " << lastShapeID << " with " << elements.back()->getNumElements()  << " when beginning " << id << std::endl;
	    }
	    shapeEnd(lastShapeID);
	} 
	elements.push_back(std::shared_ptr<Composite>(new Composite(id, attr,drawConvexHull)));
	inComposite=true;
    }

    void shapeEnd(const std::string &id) {
	dbg("Drawing.shapeEnd",3) << "end shape " << id << std::endl;
	if (!inComposite) {
	    dbg("Drawing.shapeEnd",1) << "Was not in a composite -- ignoring shapeEnd(" << id << ")" << std::endl;
	    return;
	} 
	if (elements.back()->getShapeID()->getID() != id) 
	    dbg("Drawing.shapeEnd",0) << "shapeEnd(" << id << "), but current shape is " << elements.back()->getShapeID()->getID() << std::endl;
	inComposite=false;
    }

    // Append another drawing to this one 
    void append(const Drawing &d) {
	for (int i=0;i<d.elements.size();i++)
	    elements.push_back(d.elements[i]);
    }

    // Append a composite
    void append( std::shared_ptr<Composite> c) {
	elements.push_back(c);
    }

    // Add a circle to current drawing
    void drawCircle(Point center, float r, Color c) {
	dbg("Drawing.drawCircle",2) << "center=" << center << ", radius=" << r << ", color=" << c << std::endl;
	append(std::shared_ptr<Primitive>(new Circle(center,r,c)));
    }

    // Add a polygon  to current drawing
    void drawPolygon(const std::vector<Point> &pts, Color c) {
	append(std::shared_ptr<Primitive>(new Polygon(pts,c)));
    }

    // Add a arc to current drawing
    void drawArc(Point center, Point p, float angle, Color c) {
	dbg("Drawing.drawArc",2) << "center=" << center << ", Point =" << p << ", angleCW=" << angle << ", color=" << c << std::endl;
	append(std::shared_ptr<Primitive>(new Arc(center,p,angle,c)));
    }

    // Add a line to current drawing
    void drawLine(Point p1, Point p2, Color c) {
	dbg("Drawing.drawLine",4) << "p1=" << p1 << ", p2=" << p2 << ", color=" << c << std::endl;
	append(std::shared_ptr<Primitive>(new Line(p1,p2,c)));
    }

    // Add a cubic to current drawing
    void drawCubic(Point p1, Point p2, Point p3, Point p4, Color c) {
	std::vector<Point> pts(4);
	pts[0]=p1;pts[1]=p2;pts[2]=p3;pts[3]=p4;
	append(std::shared_ptr<Primitive>(new Cubic(pts,c)));
    }

    void drawPath(std::vector<Point> pts, Color c) {
	append(std::shared_ptr<Primitive>(new Path(pts,c)));
    }
};
