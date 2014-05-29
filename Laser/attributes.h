#pragma once

#include "point.h"
#include "Simplex.hpp"

// An attribute that can be applied to a drawing primitive

class Attribute {
 protected:
    float value;
public:
    Attribute(float v) { 
	value=v;
    }
    // Apply attribute to a point by moving the point
    // p is the original location, relpos is a value that goes from 0.0 to 1.0 along path or element to make noise relative to position
    // Default implementation is no movement
    virtual Point apply(Point p, float relpos) const { return p; }
};

// An attribute which moves vertices of the segmented lines
class PointMovement: public Attribute {
public:
    PointMovement(float v): Attribute(v) { ; }

    Point apply(Point p, float relpos) const {
	double noise1=Simplex::noise(p.X(),p.Y())*value;
	double noise2=Simplex::noise(p.X(),-p.Y())*value;
	dbg("PointMovement.apply",2) << "noise=[" << noise1 << "," << noise2 << "]" << std::endl;
	return Point(p.X()+noise1,p.Y()+noise2);
    }
};

class Attributes {
    std::vector<std::shared_ptr<Attribute> > attrs;
 public:
    Attributes() { ; }
    void add(std::shared_ptr<Attribute> a) { attrs.push_back(a); }
    void add(std::string name, float value) {
	if (name== "pointmovement")
	    add(std::shared_ptr<Attribute>(new PointMovement(value)));
	else
	    dbg("Attributes.add",1) << "Bad attribute: " << name << std::endl;
    }
    void clear() { attrs.clear(); }
    // Apply all attributes to a point position
    Point apply(Point p, float relpos) const {
	for (int i=0;i<attrs.size();i++) 
	    p=attrs[i]->apply(p,relpos);
	return p;
    }
};

