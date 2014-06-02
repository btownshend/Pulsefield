#pragma once

#include <map>
#include "lo/lo.h"
#include "point.h"
#include "drawing.h"

class Leg {
    Point position;
 public:
    Leg() {;}
    void set(Point pos) {
	position=pos;
    }
    Point get() const { return position; }
};


class Person {
    int id;
    Point position;
    Leg legs[2];
    float legDiam, legSep;
    int gid, gsize;
    int age; 	// Age counter -- reset whenever something is set, increment when aged
    Attributes attributes;
 public:
    Person(int _id) {id=_id; age=0;}
    void incrementAge() {
	age++;
    }
    int getAge() const { return age; }
    void set(Point pos) {
	position=pos;
	age=0;
    }
    Point get() const { return position; }
    void setLeg(int leg,Point pos) {
	legs[leg].set(pos);
	age=0;
    }
    Point getLeg(int leg) { return legs[leg].get(); }
    void setStats(float _legDiam, float _legSep) {
	legDiam=_legDiam;
	legSep=_legSep;
    }
    void setGrouping(int _gid, int _gsize) {
	gid=_gid;
	gsize=_gsize;
    }
    float getLegDiam() const { return legDiam; }
    float getLegSep() const { return legSep; }
    int getGroupID() const { return gid; }
    int getGroupSize() const { return gsize; }

    void draw(Drawing &d) const ;
    void set(std::string key, float value, float time) {
	if (value==0) {
	    dbg("Person.set",1) << "Removing " << key << " from " << id << std::endl;
	    attributes.erase(key);
	} else
	    attributes.set(key,Attribute(value,time));
	age=0;
    }
    Attributes getAttributes() const { return attributes; }
};

class People {
    static const int MAXAGE=10;
    static People *theInstance;   // Singleton
    std::map<int,Person> p;
    Person *getOrCreatePerson(int id);
    Person *getPerson(int id);

    People() {;}
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    void incrementAge_impl();
    void draw_impl(Drawing &d) const;
public:
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static People *instance() {
	if (theInstance == NULL)
	    theInstance=new People();
	return theInstance;
    }
    static bool personExists(int id)  {
	return instance()->p.count(id)>0;
    }
    static void incrementAge() { instance()->incrementAge_impl(); }
    // Image onto drawing
    static void draw(Drawing &d)  { instance()->draw_impl(d); }
    // Set the drawing commands to image a person rather than using internal drawing routines
    static void setVisual(int uid, const Drawing &d) {
	// Not used -- always draw with internal routines
	// p[uid].setVisual(d);
    }
    std::vector<int> getIDs() const {
	std::vector<int> result;
	for (std::map<int,Person>::const_iterator a=p.begin(); a!=p.end();a++) {
	    result.push_back(a->first);
	}
	std::sort(result.begin(),result.end());
	return result;
    }

    Attributes getAttributes(int uid) const { 
	return p.at(uid).getAttributes();
    }
};
