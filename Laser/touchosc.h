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
    bool useValue, enabled;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Fader.serialize",1) << "Saving fader " << name << " settings" << std::endl;
	ar & name;
	ar & pos;
	ar & value;
	ar & enabled;
	ar & useValue;
    }
public:
    Fader() {;}
    Fader(std::string _name, unsigned int _pos, float _value=0, bool _enabled=true, bool _useValue=false):name(_name) {
	pos=_pos; value=_value; enabled=_enabled; useValue=_useValue;
    }
    const std::string &getName() const { return name; }
    float get() const { return value; }
    unsigned int getPos() const { return pos; }
    bool isEnabled() const { return enabled; }
    bool isUseValue() const { return useValue; }
    void set(float _value) { value=_value; }
    void setUseValueToggle(bool t) { useValue=t; }
    void setEnableToggle(bool t) { enabled=t; }
    int sendOSC(lo_address &dest) const {
	std::string faderspath=std::string("/ui/conn/faders/")+std::to_string(pos+1);
	std::string togglespath=std::string("/ui/conn/toggles/")+std::to_string(pos+1);
	if (lo_send(dest,(faderspath+"/label").c_str(),"s",name.c_str()) < 0)
	    return -1;
	if (lo_send(dest,faderspath.c_str(),"f",value) < 0)
	    return -1;
	dbg("Fader.sendOSC",1) << "Set fader " << pos+1 << " to " << value << std::endl;
	if (lo_send(dest,(togglespath+"/1").c_str(),"f",useValue?1.0:0.0) < 0)
	    return -1;
	dbg("Fader.sendOSC",1) << "Set useValue toggle " << pos+1 << " to " << useValue << std::endl;
	if (lo_send(dest,(togglespath+"/2").c_str(),"f",enabled?1.0:0.0) < 0)
	    return -1;
	dbg("Fader.sendOSC",1) << "Set enable toggle " << pos+1 << " to " << enabled << std::endl;
	return 0;
    }
};

// Settings for one particular selected group (e.g. connection type)
class Setting {
    static const int MAXFADERS=12;
    friend class boost::serialization::access;
    unsigned int pos; // Position in selection grid
    std::string groupName;
    std::vector<Fader> faders;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Setting.serialize",1) << "Saving " << groupName << " with " << faders.size() << " faders" << std::endl;
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
    int sendOSC(lo_address &dest) const {
	if (lo_send(dest,"/ui/conn/selected","s",groupName.c_str()) < 0)
	    return -1;
	// Set selection
	std::string selpath="/ui/conn/select/"+std::to_string(pos%4+1)+"/"+std::to_string(pos/4+1);
	dbg("Setting.sendOSC",1) << "Sending " << selpath << "," << 1.0f << " to " << lo_address_get_url(dest) << std::endl;
	if (lo_send(dest,selpath.c_str(),"f",1.0f) <0 )
	    return -1;

	// Clear all the current labels
	for (unsigned int i=0;i<MAXFADERS;i++) {
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
	if (pos>=MAXFADERS) {
	    dbg("Setting.addFader",1) << "Too many faders;  have " << faders.size()+1 << ", max=" << MAXFADERS << std::endl;
	}
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
    static const int MAXGROUPS=16;
    friend class boost::serialization::access;
    std::vector<Setting> settings;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Settings.serialize",1) << "Saving " << settings.size() << " settings" << std::endl;
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
    const Setting *getSetting(unsigned int pos) const {
	for (unsigned int i=0;i<settings.size();i++) {
	    if (pos==settings[i].getPos())
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
    void sendOSC(lo_address dest,int selectedGroup) const;
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
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Fader *getFader_impl(std::string groupName, std::string faderName);
    void frameTick_impl(int frame);
    struct timeval pressTime;   // Time that a button was pressed (to check if it was held for a long time)
    int trackUID1,trackUID2;  // UIDs tracked in TouchOSC
    bool bodyEnabled, legsEnabled;
 public:
    static TouchOSC *instance() {
	if (theInstance == NULL)
	    theInstance=new TouchOSC();
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static Fader *getFader(std::string groupName, std::string faderName) { return instance()->getFader_impl(groupName,faderName); }
    // Get setting of fader applying enabled and useValue settings
    static float getValue(std::string groupName, std::string faderName, float groupValue, float offValue) {
	Fader *f=getFader(groupName,faderName);
	if (!f->isEnabled())
	    return offValue;
	if (f->isUseValue())
	    return f->get()*groupValue;
	else
	    return f->get();
    }
    void save(std::string filename) const;
    void load(std::string filename);
    // Do anything needed on frame ticks
    static void frameTick(int frame) { instance()->frameTick_impl(frame); }
    void updateConnectionMap() const;

    void toggleBody() { bodyEnabled=!bodyEnabled; }
    bool isBodyEnabled() { return bodyEnabled; }
    void toggleLegs() { legsEnabled=!legsEnabled; }
    bool isLegsEnabled() { return legsEnabled; }
};



