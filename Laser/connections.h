#pragma once
#include <iostream>
#include <map>
#include <string>
#include "lo/lo.h"
#include "dbg.h"
#include "drawing.h"

// Keep track of conductor connections

typedef std::string CIDType;

class ConnectionAttribute {
    float value;
    float time;
 public:
    ConnectionAttribute() {;}
    ConnectionAttribute(float _value, float _time) { value=_value; time=_time;}
    float getValue() const { return value; }
    float getTime() const { return time; }
    friend std::ostream &operator<<(std::ostream &s, const ConnectionAttribute &c);
};

class Connection {
    CIDType cid;
    int uid[2];
    std::map<std::string,ConnectionAttribute> attributes;
    int age; 	// Age counter -- reset whenever something is set, increment when aged
    Drawing visual; 	// Drawing used to image this connection
 public:
    Connection() {age=0;}
    Connection(CIDType _cid, int uid1, int uid2) {
	cid=_cid;
	uid[0]=uid1;
	uid[1]=uid2;
	age=0;
    }
    void set(std::string type, std::string subtype, float value, float time) {
	std::string key=type+"."+subtype;
	if (value==0) {
	    dbg("Connection.set",1) << "Removing " << key << " from " << cid << std::endl;
	    attributes.erase(key);
	} else
	    attributes[key]=ConnectionAttribute(value,time);
	age=0;
    }
    ConnectionAttribute get(std::string key) {
	return attributes[key];
    }
    void incrementAge() {
	age++;
    }
    int getAge() const { return age; }
    void setVisual(const Drawing &d) { visual=d; }
    void draw(Drawing &d) const {  d.append(visual); }
    friend std::ostream &operator<<(std::ostream &s, const Connection &c);
};

class Connections {
    static const int MAXAGE=10;   // Number of frames before unupdated connections should be flushed
    static Connections *theInstance;   // Singleton
    std::map<CIDType,Connection> conns;

    Connections() {;}
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    void incrementAge_impl();
    void draw_impl(Drawing &d) const;
 public:
    static Connections *instance() {
	if (theInstance == NULL)
	    theInstance=new Connections();
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static void incrementAge() { instance()->incrementAge_impl(); }
	
    static void draw(Drawing &d)  { instance()->draw_impl(d); }

    // Set the drawing commands to image a person rather than using internal drawing routines
    static void setVisual(CIDType cid, const Drawing &d) {
	instance()->conns[cid].setVisual(d);
    }
    
    friend std::ostream &operator<<(std::ostream &s, const Connections &c);
};
