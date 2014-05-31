#pragma once

#include "point.h"
#include "Simplex.hpp"
#include "touchosc.h"

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
	float v=TouchOSC::getFader("grouped","amplitude")->get() * 0.5;
	float s=TouchOSC::getFader("grouped","scale")->get() * 5;
	float ph=TouchOSC::getFader("grouped","phase")->get()*1.0;
	float tx=TouchOSC::getFader("grouped","temporalx")->get()*4.0;
	float ty=TouchOSC::getFader("grouped","temporaly")->get()*4.0;
	dbg("PointMovement",10) << "Value=" << v << ", Scale=" << s << ", Phase=" << ph << ", Temporal=" << tx << "," << ty << std::endl;
	Point np=p*s+ph;
	struct timeval now;
	gettimeofday(&now,0);
	np=np+Point(tx,ty)*(now.tv_sec%1000+now.tv_usec/1e6);
	double noise1=Simplex::noise(np.X()*s,np.Y()*s)*v;
	double noise2=Simplex::noise(np.X(),-np.Y())*v;
	dbg("PointMovement.apply",10) << "noise=[" << noise1 << "," << noise2 << "]" << std::endl;
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

