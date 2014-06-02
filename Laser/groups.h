#pragma once

#include <map>
#include "lo/lo.h"
#include "drawing.h"

class Group {
    int id;
    Point position;
    float diameter;
    std::set<int> ids;
    int age; 	// Age counter -- reset whenever something is set, increment when aged
    Attributes attributes;
 public:
    Group(int _id) {id=_id; diameter=0; position=Point(0,0); age=0; set("allgrps",1.0,0.0); }
    void incrementAge() {
	age++;
    }
    int getAge() const { return age; }
    void set(Point pos) {
	position=pos;
	age=0;
    }
    void setDiameter(float diam) {
	diameter=diam;
    }
    Point get() const { return position; }
    int getGroupID() const { return id; }
    int getGroupSize() const { return ids.size(); }
    const std::set<int> &getPeople() const { return ids; }
    void addPerson(int uid) { ids.insert(uid); }
    void removePerson(int uid) { ids.erase(uid); }
    void draw(Drawing &d) const ;
    void set(std::string key, float value, float time) {
	if (value==0) {
	    dbg("Group.set",1) << "Removing " << key << " from " << id << std::endl;
	    attributes.erase(key);
	} else
	    attributes.set(key,Attribute(value,time));
	age=0;
    }
    Attributes getAttributes() const { return attributes; }
};

class Groups {
    static const int MAXAGE=10;
    static Groups *theInstance;   // Singleton
    std::map<int,Group> p;

    Groups() {;}
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    void incrementAge_impl();
    void draw_impl(Drawing &d) const;
public:
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static Groups *instance() {
	if (theInstance == NULL)
	    theInstance=new Groups();
	return theInstance;
    }
    static bool groupExists(int id)  {
	return instance()->p.count(id)>0;
    }
    static void incrementAge() { instance()->incrementAge_impl(); }
    // Image onto drawing
    static void draw(Drawing &d)  { instance()->draw_impl(d); }
    // Set the drawing commands to image a group rather than using internal drawing routines
    static void setVisual(int uid, const Drawing &d) {
	// Not used -- always draw with internal routines
	// p[uid].setVisual(d);
    }
    Group *getOrCreateGroup(int id);
    Group *getGroup(int id);

    std::vector<int> getIDs() const {
	std::vector<int> result;
	for (std::map<int,Group>::const_iterator a=p.begin(); a!=p.end();a++) {
	    result.push_back(a->first);
	}
	std::sort(result.begin(),result.end());
	return result;
    }

    Attributes getAttributes(int uid) const { 
	return p.at(uid).getAttributes();
    }
};
