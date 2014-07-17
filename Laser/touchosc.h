#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <lo/lo.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "dbg.h"
#include "urlconfig.h"

// A single fader plus a toggle
// Has a name and a position
class Fader {
    friend class boost::serialization::access;
    std::string name;
    unsigned int pos;
    float value;
    bool useValue;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Fader.serialize",1) << "Saving fader " << name << " settings" << std::endl;
	ar & name;
	ar & pos;
	ar & value;
	ar & useValue;
    }
public:
    Fader() {;}
    Fader(std::string _name, unsigned int _pos, float _value=0, bool _useValue=false):name(_name) {
	pos=_pos; value=_value; useValue=_useValue;
    }
    const std::string &getName() const { return name; }
    float get() const { return value; }
    unsigned int getPos() const { return pos; }
    bool isUseValue() const { return useValue; }
    void set(float _value) { value=_value; }
    void setUseValueToggle(bool t) { useValue=t; }
    int sendOSC() const;
};

class Button {
    friend class boost::serialization::access;
    std::string name;
    unsigned int pos;
    bool enabled;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Button.serialize",1) << "Saving button " << name << " settings" << std::endl;
	ar & name;
	ar & pos;
	ar & enabled;
    }
public:
    Button() {;}
    Button(std::string _name, unsigned int _pos, bool _enabled=true):name(_name) {
	pos=_pos; enabled=_enabled; 
    }
    const std::string &getName() const { return name; }
    bool get() const { return enabled; }
    unsigned int getPos() const { return pos; }
    void set(bool t) { enabled=t; }
    int sendOSC() const;
};

// Settings for one particular selected group (e.g. connection type)
class Setting {
    static const int MAXFADERS=18;
    friend class boost::serialization::access;
    unsigned int pos; // Position in selection grid
    std::string groupName;
    std::vector<Fader> faders;
    std::vector<Button> buttons;
    template <class Archive> void serialize(Archive &ar, const unsigned int version) {
	dbg("Setting.serialize",1) << "Saving " << groupName << " with " << faders.size() << " faders" << " and " << buttons.size() << " buttons" << std::endl;
	ar & pos;
	ar & groupName;
	ar & faders;
	ar & buttons;
    }
public:
    static const int MAXBUTTONS=7;
    Setting() {;}
 Setting(const std::string &_groupname, unsigned int _pos): groupName(_groupname), faders(),buttons() {pos=_pos; }

    const std::string &getGroupName() const { return groupName; }
    unsigned int getPos() const { return pos; }
    // Send OSC to make UI reflect current values
    int sendOSC() const;

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

    // Add a button
    void addButton(const std::string &name, unsigned int pos=-1, bool t=false) {
	if (pos==-1)
	    pos=buttons.size();
	if (pos>=MAXBUTTONS) {
	    dbg("Setting.addButton",1) << "Too many buttons;  have " << buttons.size()+1 << ", max=" << MAXBUTTONS << std::endl;
	}
	buttons.push_back(Button(name,pos,t));
    }

    // Get button by name
    Button *getButton(const std::string &name) {
	for (unsigned int i=0;i<buttons.size();i++) {
	    if (name==buttons[i].getName())
		return &buttons[i];
	}
	return NULL;
    }
    // Get button by position
    Button *getButton(unsigned int pos) {
	for (unsigned int i=0;i<buttons.size();i++) {
	    if (pos==buttons[i].getPos())
		return &buttons[i];
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
	dbg("Settings.serialize",1) << "Saving " << settings.size() << " settings" << "; first group=" << settings[0].getGroupName() << std::endl;
	ar & settings;
    }
public:
    Settings() {; }
    void addGroup( const std::string &gname) {
	int newPos=0;
	for (int i=0;i<settings.size();i++)
	    if (settings[i].getPos() >= newPos)
		newPos=settings[i].getPos()+1;
	settings.push_back(Setting(gname,newPos));
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
    void sendOSC(int selectedGroup) const;
};

class TouchOSC {
    static TouchOSC *theInstance;   // Singleton
    lo_address remote;
    Settings settings;
    unsigned int selectedGroup;
    void sendOSC() const;
    // TouchOSC UI
    TouchOSC();
    ~TouchOSC();
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Fader *getFader_impl(std::string groupName, std::string faderName);
    Button *getButton_impl(std::string groupName, std::string optionName);
    void frameTick_impl(int frame);
    struct timeval pressTime;   // Time that a button was pressed (to check if it was held for a long time)
    int trackUID1,trackUID2;  // UIDs tracked in TouchOSC
    bool frozen,layeringEnabled,onePerEnabled,fusionEnabled,attrsEnabled;
    int maxConnections;
    float visualThreshold,conductorGlobal;
 public:
    static TouchOSC *instance() {
	if (theInstance == NULL) {
	    new TouchOSC();
	    assert(theInstance != NULL);
	    // Need to be careful of this being re-entrant into instance()
	    theInstance->load("settings_default.txt");
	}
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    static Fader *getFader(std::string groupName, std::string faderName) { return instance()->getFader_impl(groupName,faderName); }
    static Button *getButton(std::string groupName, std::string optionName) { return instance()->getButton_impl(groupName,optionName); }
    // Get setting of fader applying useValue settings
    static float getValue(std::string groupName, std::string faderName, float groupValue, float offValue) {
	Fader *f=getFader(groupName,faderName);
	if (f->isUseValue())
	    return f->get()*groupValue;
	else
	    return f->get();
    }
    static float getEnabled(std::string groupName, std::string optionName) {
	Button *e=getButton(groupName,optionName);
	return e->get();
    }
    bool isFrozen() const { return frozen; }
    void save(std::string filename) const;
    void load(std::string filename);
    // Do anything needed on frame ticks
    static void frameTick(int frame) { instance()->frameTick_impl(frame); }
    void updateConnectionMap() const;

    bool isLayeringEnabled() const { return layeringEnabled; }
    bool isOnePerEnabled() const { return onePerEnabled; }
    bool isFusionEnabled() const { return fusionEnabled; }
    bool isAttrsEnabled() const { return attrsEnabled; }
    int getMaxConnections() const { return maxConnections; }
    float getVisualThreshold() const { return visualThreshold; }
    float getConductorGlobal() const { return conductorGlobal; }
    int send(std::string path, float value) const;
    int send(std::string path, std::string value) const;
};
