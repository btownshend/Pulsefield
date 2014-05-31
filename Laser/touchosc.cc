#include <fstream>
#include "touchosc.h"

const char *TouchOSC::PORT="9998";

TouchOSC *TouchOSC::theInstance=NULL;

TouchOSC::TouchOSC()  {
    // Start off connected to localhost (for debugging)
    remote=lo_address_new("localhost",PORT);
    local=lo_address_new("localhost",PORT);

    // Setup 
    currentPos=0;
    settings.addGroup("friends",currentPos++);
    selectedGroup=0;
    activityLED=true;
    settings.addGroup("grouped",currentPos++);

    for (int pos=0;pos<16;pos++) {
	Setting *s=settings.getSetting(pos);
	if (s==NULL)
	    continue;
	s->addFader("amplitude",-1,0.5);
	s->addFader("scale",-1,0.5);
	s->addFader("phase",-1,0.5);
	//	for (int i=3;i<7;i++)
	// s->addFader("",i);
    }

    // Send out current settings
    sendOSC();
}
		       
TouchOSC::~TouchOSC() {
    lo_address_free(remote);
}

void TouchOSC::sendOSC() {
    activityLED=!activityLED;
    sendOSC(local);
    sendOSC(remote);
}

void TouchOSC::sendOSC(lo_address dest) {
    // Send OSC to make UI reflect current values
    dbg("TouchOSC.sendOSC",1) << "Sending OSC updates with group " << selectedGroup << " to " << dest << std::endl;
    Setting *s=settings.getSetting(selectedGroup);
    if (s==NULL) {
	dbg("TouchOSC.sendOSC",1) << "Selected group " << selectedGroup << " is not valid, switching to group 0" << std::endl;
	selectedGroup=0;
	s=settings.getSetting(selectedGroup);
	if (s==NULL)
	    return;
    }
	
    if (lo_send(dest,"/ui/active1","f",activityLED?1.0f:0.0f) <0 ) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of /ui/active1 to " << lo_address_get_url(dest) << ": " << lo_address_errstr(dest) << std::endl;
	return;
    }
    if (lo_send(dest,"/ui/active2","f",activityLED?0.0f:1.0f) <0 ) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of /ui/active2 to " << lo_address_get_url(dest) << ": " << lo_address_errstr(dest) << std::endl;
	return;
    }
    if(s->sendOSC(dest) <0) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of OSC data to " << lo_address_get_url(dest) << ": " << lo_address_errstr(dest) << std::endl;
	return;
    }
}

Fader *TouchOSC::getFader(std::string groupName, std::string faderName) {
    Setting *s=settings.getSetting(groupName);
    if (s==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested group " << groupName << " does not exists; adding." << std::endl;
	settings.addGroup(groupName,currentPos++);
	s=settings.getSetting(groupName);
    }
    Fader *f=s->getFader(faderName);
    if (f==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested fader " << faderName << " does not exist in group " << groupName << " adding it" << std::endl;
	s->addFader(faderName,-1,0.5f);
	f=s->getFader(faderName);
    }
    return f;
}

int TouchOSC::handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("TouchOSC.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;

    std::string host=lo_address_get_hostname(lo_message_get_source(msg));
    if (host != lo_address_get_hostname(remote)) {
	lo_address_free(remote);
	remote=lo_address_new(host.c_str(),PORT);
	dbg("TouchOSC.handleOSCMessage",1)  << "Set remote to " << lo_address_get_url(remote) << std::endl;
	sendOSC();
    }
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"ui")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"conn")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"select")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("TouchOSC.handleOSCMessage",1) << "Selected group position " << col << "/" << row << " with value " << argv[0]->f << std::endl;
		if (argv[0]->f>0.5) {
		    int gpos=row*4+col;
		    Setting *newGroup=settings.getSetting(gpos);
		    if (newGroup==NULL) {
			dbg("TouchOSC.handleOSCMessage",1) << "No group at position " << gpos << std::endl;
		    } else {
			selectedGroup=gpos;
			dbg("TouchOSC.handleOSCMessage",1) << "Switch to group " << newGroup->getGroupName() << std::endl;
			sendOSC();
		    }
		}
		handled=true;
	    } else if (strcmp(tok,"faders")==0) {
		int pos=atoi(strtok(NULL,"/"))-1;
		dbg("TouchOSC.handleOSCMessage",1) << "Set fader " << pos << " with value " << argv[0]->f << std::endl;
		Setting *s=settings.getSetting(selectedGroup);
		if (s==NULL) {
		    dbg("TouchOSC.handleOSCMessage",1) << "Selected group " << selectedGroup << " is not valid." << std::endl;
		} else {
		    Fader *f=s->getFader(pos);
		    if (f==NULL) {
			dbg("TouchOSC.handleOSCMessage",1) << "Selected fader " << pos << " is not valid." << std::endl;
		    } else {
			f->set(argv[0]->f);
		    }
		}
		handled=true;
	    } else if (strcmp(tok,"toggles")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("TouchOSC.handleOSCMessage",1) << "Set toggle " << col << "/" << row << " with value " << argv[0]->f << std::endl;
		Setting *s=settings.getSetting(selectedGroup);
		if (s==NULL) {
		    dbg("TouchOSC.handleOSCMessage",1) << "Selected group " << selectedGroup << " is not valid." << std::endl;
		} else {
		    Fader *f=s->getFader(col);
		    if (f==NULL) {
			dbg("TouchOSC.handleOSCMessage",1) << "Selected fader " << col << " is not valid." << std::endl;
		    } else {
			f->setToggle(argv[0]->f>0.5);
		    }
		}
		handled=true;
	    }
	} else if (strcmp(tok,"save")==0) {
	    if (argv[0]->f > 0.5)
		save("settings.txt");
	    handled=true;
	} else if (strcmp(tok,"load")==0) {
	    if (argv[0]->f > 0.5)
		load("settings.txt");
	    handled=true;
	}
    }
    if (!handled) {
	dbg("TouchOSC.handleOSCMessage",1) << "Unhanded message: " << path << ": parse failed at token: " << tok << std::endl;
    }
    
    delete [] pathCopy;
    return handled?0:1;
}

void TouchOSC::save(std::string filename) const {
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << settings;
    dbg("TouchOSC.save",1) << "Saved settings in " << filename << std::endl;
}

void TouchOSC::load(std::string filename) {
    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> settings;
    dbg("TouchOSC.save",1) << "Loaded settings from " << filename << "; now have " << settings.size() << "groups" << std::endl;
    sendOSC();
}
