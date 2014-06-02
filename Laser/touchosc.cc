#include <fstream>
#include <sys/time.h>
#include "touchosc.h"
#include "person.h"
#include "connections.h"

const char *TouchOSC::PORT="9998";

TouchOSC *TouchOSC::theInstance=NULL;

TouchOSC::TouchOSC()  {
    // Start off connected to localhost (for debugging)
    remote=lo_address_new("localhost",PORT);
    local=lo_address_new("localhost",PORT);

    // Setup 
    currentPos=0;
    selectedGroup=0;
    activityLED=true;
    legsEnabled=true;
    bodyEnabled=false;

    load("settings-default.txt");
    // Send out current settings
    sendOSC();
}
		       
TouchOSC::~TouchOSC() {
    lo_address_free(remote);
}

void TouchOSC::sendOSC() {
    sendOSC(local);
    sendOSC(remote);
}

void Settings::sendOSC(lo_address dest,int selectedGroup) const {
    // Send labels for attribute selection
    for (int p=0;p<MAXGROUPS;p++) {
	int col=(p%4)+1;
	int row=p/4 + 1;
	std::string path="/ui/conn/select/"+std::to_string(col)+"/"+std::to_string(row);
	//  set label
	std::string label="";
	const Setting *s=getSetting(p);
	if (s!=NULL) {
	    label=s->getGroupName();
	}
	dbg("Setting.sendOSC",3) << "Sending " << path << "," << label << " to " << lo_address_get_url(dest) << std::endl;
	if (lo_send(dest,(path+"/label").c_str(),"s",label.c_str()) <0 ) {
	    dbg("TouchOSC.sendOSC",1) << "Failed send of " << path << "  to " << lo_address_get_url(dest) << ": " << lo_address_errstr(dest) << std::endl;
	    return;
	}
    }

    // Send settings for current select
    const Setting *s= getSetting(selectedGroup);
    if (s==NULL) {
	dbg("TouchOSC.sendOSC",1) << "Selected group " << selectedGroup << " is not valid, switching to group 0" << std::endl;
	s=getSetting(0);
	if (s==NULL)
	    return;
    }
    if(s->sendOSC(dest) <0) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of OSC data to " << lo_address_get_url(dest) << ": " << lo_address_errstr(dest) << std::endl;
	return;
    }
}

void TouchOSC::sendOSC(lo_address dest) {
    // Send OSC to make UI reflect current values
    dbg("TouchOSC.sendOSC",1) << "Sending OSC updates with group " << selectedGroup << " to " << dest << std::endl;
    settings.sendOSC(dest,selectedGroup);

}

void TouchOSC::frameTick_impl(int frame) {
    activityLED=!activityLED;
    if (lo_send(remote,"/ui/active1","f",activityLED?1.0f:0.0f) <0 ) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of /ui/active1 to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
    if (lo_send(remote,"/ui/active2","f",activityLED?0.0f:1.0f) <0 ) {
	dbg("TouchOSC.sendOSC",1) << "Failed send of /ui/active2 to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
    updateConnectionMap();
}

Fader *TouchOSC::getFader_impl(std::string groupName, std::string faderName) {
    bool updatedUI=false;
    Setting *s=settings.getSetting(groupName);
    if (s==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested group " << groupName << " does not exists; adding." << std::endl;
	settings.addGroup(groupName,currentPos++);
	s=settings.getSetting(groupName);
	updatedUI=true;
    }
    Fader *f=s->getFader(faderName);
    if (f==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested fader " << faderName << " does not exist in group " << groupName << " adding it" << std::endl;
	s->addFader(faderName,-1,0.5f);
	f=s->getFader(faderName);
	updatedUI=true;
    }
    if (updatedUI)
	sendOSC();
    return f;
}

int TouchOSC::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
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
		    const Setting *newGroup=settings.getSetting(gpos);
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
			if (row==0)
			    f->setUseValueToggle(argv[0]->f>0.5);
			else if (row==1)
			    f->setEnableToggle(argv[0]->f>0.5);
			else
			    dbg("TouchOSC.handleOSCMessage",1) << "Bad row: " << row << " from /ui/conn/toggles" << std::endl;
		    }
		}
		handled=true;
	    }
	} else if (strcmp(tok,"preset")==0) {
	    tok=strtok(NULL,"/");
	    struct timeval now;
	    gettimeofday(&now,0);
	    if (argv[0]->f > 0.5)
		pressTime=now;
	    else {
		if (pressTime.tv_sec==0) {
		    dbg("TouchOSC.handleOSCMessage",1) << "Missed preset " << tok << " press" << std::endl;
		} else {
		    float delta=(now.tv_sec-pressTime.tv_sec)+(now.tv_usec-pressTime.tv_usec)/1e6;
		    dbg("TouchOSC.handleOSCMessage",1) << "Got preset " << tok << " held for " << delta << " seconds" << std::endl;
		    if (delta>1) 
			save(std::string("settings_")+tok+".txt");
		    else
			load(std::string("settings_")+tok+".txt");
		    pressTime.tv_sec=0;
		}
	    }
	    handled=true;
	} else if (strcmp(tok,"body")==0) {
	    bodyEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"legs")==0) {
	    legsEnabled=argv[0]->f>0.5;
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
    try {
	std::ofstream ofs(filename);
	boost::archive::text_oarchive oa(ofs);
	oa << settings;
	dbg("TouchOSC.save",1) << "Saved settings in " << filename << std::endl;
    } catch (std::ofstream::failure e) {
	std::cerr << "Exception saving to " << filename << ": " << e.what() << std::endl;
    } catch (boost::archive::archive_exception e) {
	std::cerr << "Exception saving to " << filename << ": " << e.what() << std::endl;
    }
}

void TouchOSC::load(std::string filename) {
    try {
	std::ifstream ifs(filename);
	boost::archive::text_iarchive ia(ifs);
	ia >> settings;
	dbg("TouchOSC.save",1) << "Loaded settings from " << filename << "; now have " << settings.size() << "groups" << std::endl;
	sendOSC();
    } catch (std::ifstream::failure e) {
	std::cerr << "Exception loading from " << filename << ": " << e.what() << std::endl;
    } catch (boost::archive::archive_exception e) {
	std::cerr << "Exception loading from " << filename << ": " << e.what() << std::endl;
    }
}

// Send OSC to touchOSC to update connections
// UID labels at /ui/uid/[row]
// multipush at /ui/linked/[col]/[row]
void TouchOSC::updateConnectionMap() const {
    // Update UID labels
    static const int MAXUIDDISPLAY=10;
       std::vector<int> uids = People::instance()->getIDs();
    for (int i=0;i<MAXUIDDISPLAY;i++) {
	std::string uidstring = "";
	if (i<uids.size())
	    uidstring = std::to_string(uids[i]);
	if (lo_send(remote,("/ui/uid/"+std::to_string(i+1)).c_str(),"s",uidstring.c_str()) <0 ) {
	    dbg("TouchOSC.updateConnectionMap",1) << "Failed send of /ui/uid to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return;
	}
	for (int j=0;j<MAXUIDDISPLAY;j++) {
	    bool connected=false;
	    if (i<uids.size() && j<uids.size())
		connected = Connections::instance()->isConnected(uids[i],uids[j]) || Connections::instance()->isConnected(uids[i],uids[j]);
	    std::string path="/ui/linked/"+std::to_string(j+1)+"/"+std::to_string(i+1);
	    dbg("TouchOSC.updateConnectionMap",3) << "Send " << path << "," << (connected?1.0f:0.0f) << std::endl;

	    if (lo_send(remote,path.c_str(),"f",connected?1.0f:0.0f) <0 ) {
		dbg("TouchOSC.updateConnectionMap",1) << "Failed send of /ui/linked to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
		return;
	    }
	}
    }
    
}
