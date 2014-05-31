#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <lo/lo.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "dbg.h"

// A single fader plus a toggle
// Has a name and a position
class Fader {
    friend class boost::serialization::access;
    std::string name;
    unsigned int pos;
    float value;
    bool toggle;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	ar & name;
	ar & pos;
	ar & value;
	ar & toggle;
    }
public:
    Fader() {;}
    Fader(std::string _name, unsigned int _pos, float _value=0, bool t=false):name(_name) { pos=_pos; value=_value; toggle=t;}
    const std::string &getName() const { return name; }
    float get() const { return value; }
    unsigned int getPos() const { return pos; }
    bool isToggleOn() const { return toggle; }
    void set(float _value) { value=_value; }
    void setToggle(bool t) { toggle=t; }
    int sendOSC(lo_address &dest) {
	std::string faderspath=std::string("/ui/conn/faders/")+std::to_string(pos+1);
	std::string togglespath=std::string("/ui/conn/toggles/")+std::to_string(pos+1)+"/1";
	if (lo_send(dest,(faderspath+"/label").c_str(),"s",name.c_str()) < 0)
	    return -1;
	if (lo_send(dest,faderspath.c_str(),"f",value) < 0)
	    return -1;
	std::cout << "Set fader " << pos+1 << " to " << value << std::endl;
	if (lo_send(dest,togglespath.c_str(),"f",toggle?1.0:0.0) < 0)
	    return -1;
	std::cout << "Set toggle " << pos+1 << " to " << toggle << std::endl;
	return 0;
    }
};

// Settings for one particular selected group (e.g. connection type)
class Setting {
    friend class boost::serialization::access;
    unsigned int pos; // Position in selection grid
    std::string groupName;
    std::vector<Fader> faders;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	ar & pos;
	ar & groupName;
	ar & faders;
    }
public:
    Setting() {;}
    Setting(const std::string &_groupname, unsigned int _pos): groupName(_groupname), faders() {pos=_pos; }

    const std::string &getGroupName() const { return groupName; }
    unsigned int getPos() const { return pos; }
    // Send OSC to make UI reflect current values
    int sendOSC(lo_address &dest) {
	if (lo_send(dest,"/ui/conn/selected","s",groupName.c_str()) < 0)
	    return -1;
	// Clear all the current labels
	for (unsigned int i=0;i<7;i++) {
	    std::string faderspath=std::string("/ui/conn/faders/")+std::to_string(i+1);
	    if (lo_send(dest,faderspath.c_str(),"f",0.0f) < 0)
		return -1;
	    if (lo_send(dest,(faderspath+"/label").c_str(),"s","") < 0)
		return -1;
	}
	    
	for (unsigned int i=0;i<faders.size();i++)
	    if (faders[i].sendOSC(dest) < 0)
		return -1;
	return 0;
    }

    // Add a fader
    void addFader(const std::string &name, unsigned int pos=-1, float value=0.0, bool t=false) {
	if (pos==-1)
	    pos=faders.size();
	faders.push_back(Fader(name,pos,value,t));
    }

    // Get fader by name
    Fader *getFader(const std::string &name) {
	for (unsigned int i=0;i<faders.size();i++) {
	    if (name==faders[i].getName())
		return &faders[i];
	}
	return NULL;
    }
    // Get fader by position
    Fader *getFader(unsigned int pos) {
	for (unsigned int i=0;i<faders.size();i++) {
	    if (pos==faders[i].getPos())
		return &faders[i];
	}
	return NULL;
    }
};
    
// All the settings for all the connection types
class Settings {
    friend class boost::serialization::access;
    std::vector<Setting> settings;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	ar & settings;
    }
public:
    Settings() {; }
    void addGroup( const std::string &gname,unsigned int pos) {
	settings.push_back(Setting(gname,pos));
    }
    Setting *getSetting(const std::string &name) {
	for (unsigned int i=0;i<settings.size();i++) {
	    if (name==settings[i].getGroupName())
		return &settings[i];
	}
	return NULL;
    }
    Setting *getSetting(unsigned int pos) {
	for (unsigned int i=0;i<settings.size();i++) {
	    if (pos==settings[i].getPos())
		return &settings[i];
	}
	return NULL;
    }
    unsigned int size() const { return settings.size(); }
};

class TouchOSC {
    static TouchOSC *theInstance;   // Singleton
    static const char *PORT;
    lo_address local;
    lo_address remote;
    Settings settings;
    unsigned int selectedGroup;
    void sendOSC(lo_address dest);
    void sendOSC();
    bool activityLED;
    int currentPos;
    // TouchOSC UI
    TouchOSC();
    ~TouchOSC();
    static TouchOSC *instance() {
	if (theInstance == NULL)
	    theInstance=new TouchOSC();
	return theInstance;
    }
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Fader *getFader_impl(std::string groupName, std::string faderName);
 public:
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage(path,types,argv,argc,msg);
    }
    static Fader *getFader(std::string groupName, std::string faderName) { return instance()->getFader(groupName,faderName); }
    void save(std::string filename) const;
    void load(std::string filename);
};



