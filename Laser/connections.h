#pragma once
#include <iostream>
#include <map>
#include <string>
#include "lo/lo.h"
#include "dbg.h"
#include "drawing.h"

// Keep track of conductor connections

typedef std::string CIDType;

class Connection {
    CIDType cid;
    int uid[2];
    Attributes attributes;
    int age; 	// Age counter -- reset whenever something is set, increment when aged
    Drawing visual; 	// Drawing used to image this connection
 public:
    Connection() {age=0;uid[0]=-1;uid[1]=-1;cid="Empty"; set("test","allconx",1.0,0.0); }
    Connection(CIDType _cid, int uid1, int uid2) {
	cid=_cid;
	uid[0]=uid1;
	uid[1]=uid2;
	age=0;
    }
    CIDType getCID() const { return cid; }
    int getUID(int i) const {
	assert(i>=0 && i<=1);
	return uid[i];
    }
    void set(std::string type, std::string subtype, float value, float time) {
	std::string key=subtype;
	if (value==0) {
	    dbg("Connection.set",1) << "Removing " << key << " from " << cid << std::endl;
	    attributes.erase(key);
	} else
	    attributes.set(key,Attribute(value,time));
	age=0;
    }
    void incrementAge() {
	age++;
    }
    int getAge() const { return age; }
    void setAge(int _age)  { age=_age;}
    void setVisual(const Drawing &d) { visual=d; }
    void draw(Drawing &d) const;
    const Attributes &getAttributes() const { return attributes; }
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
    void add(const Connection &conn) {
	conns[conn.getCID()]=conn;
    }

    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static void incrementAge() { instance()->incrementAge_impl(); }
	
    static void draw(Drawing &d)  { instance()->draw_impl(d); }

    // Set the drawing commands to image a person rather than using internal drawing routines
    static void setVisual(CIDType cid, const Drawing &d) {
	instance()->conns.at(cid).setVisual(d);
    }
    
    bool connectionExists(CIDType cid) const {
	return conns.count(cid)>0;
    }

    bool isConnected(int uid1, int uid2) const {
	return conns.count(std::to_string(uid1)+"-"+std::to_string(uid2))>0 || conns.count(std::to_string(uid2)+"-"+std::to_string(uid1))>0;
    }

    const Connection &getConnection(int uid1, int uid2) const {
	if (uid1>uid2)
	    std::swap(uid1,uid2);
	return conns.at(std::to_string(uid1)+"-"+std::to_string(uid2));
    }

    friend std::ostream &operator<<(std::ostream &s, const Connections &c);
};
