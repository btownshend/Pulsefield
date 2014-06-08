#include <fstream>
#include <sys/time.h>
#include "touchosc.h"
#include "person.h"
#include "connections.h"
#include "music.h"
#include "conductor.h"

const char *TouchOSC::PORT="9998";

TouchOSC *TouchOSC::theInstance=NULL;

TouchOSC::TouchOSC()  {
    // Start off connected to localhost (for debugging)
    remote=lo_address_new("localhost",PORT);

    // Setup 
    selectedGroup=0;
    activityLED=true;
    legsEnabled=true;
    bodyEnabled=false;
    layeringEnabled=false;
    onePerEnabled=true;
    attrsEnabled=true;
    fusionEnabled=true;
    maxConnections=10;
    visualThreshold=0.0;
    conductorGlobal=1.0;
    pressTime.tv_sec=0;
    trackUID1=-1;
    trackUID2=-1;
}
		       
TouchOSC::~TouchOSC() {
    lo_address_free(remote);
}

int Fader::sendOSC() const {
    std::string faderspath=std::string("/ui/conn/faders/")+std::to_string(pos+1);
    std::string togglespath=std::string("/ui/conn/toggles/")+std::to_string(pos+1);
    if (TouchOSC::instance()->send((faderspath+"/label"),name) < 0)
	return -1;
    if (TouchOSC::instance()->send(faderspath,value) < 0)
	return -1;
    dbg("Fader.sendOSC",1) << "Set fader " << pos+1 << " to " << value << std::endl;
    if (TouchOSC::instance()->send((togglespath+"/1"),useValue?1.0:0.0) < 0)
	return -1;
    dbg("Fader.sendOSC",1) << "Set useValue toggle " << pos+1 << " to " << useValue << std::endl;
    return 0;
}

int Button::sendOSC() const {
    std::string buttonspath=std::string("/ui/conn/buttons/")+std::to_string(pos+1);
    if (TouchOSC::instance()->send((buttonspath+"/label"),name) < 0)
	return -1;
    if (TouchOSC::instance()->send(buttonspath+"/1",enabled?1.0:0.0) < 0)
	return -1;
    dbg("Button.sendOSC",1) << "Set button " << pos+1 << " to " << enabled << std::endl;
    return 0;
}

int Setting::sendOSC() const {
    if (TouchOSC::instance()->send("/ui/conn/selected",groupName) < 0)
	return -1;
    // Set selection
    std::string selpath="/ui/conn/select/"+std::to_string(pos%4+1)+"/"+std::to_string(pos/4+1);
    dbg("Setting.sendOSC",1) << "Sending " << selpath << "," << 1.0f << std::endl;
    if (TouchOSC::instance()->send(selpath,1.0f) <0 )
	return -1;

    const bool preClear = false;

    if (preClear) {
	// Clear all the current fader values and labels
	for (unsigned int i=0;i<MAXFADERS;i++) {
	    std::string faderspath=std::string("/ui/conn/faders/")+std::to_string(i+1);
	    if (TouchOSC::instance()->send(faderspath,0.0f) < 0)
		return -1;
	    if (TouchOSC::instance()->send((faderspath+"/label"),"") < 0)
		return -1;
	}
	// Clear all the current button values and labels
	for (unsigned int i=0;i<MAXBUTTONS;i++) {
	    std::string buttonspath=std::string("/ui/conn/buttons/")+std::to_string(i+1);
	    if (TouchOSC::instance()->send(buttonspath,0.0f) < 0)
		return -1;
	    if (TouchOSC::instance()->send((buttonspath+"/label"),"") < 0)
		return -1;
	}
    }

    for (unsigned int i=0;i<faders.size();i++)
	if (faders[i].sendOSC() < 0)
	    return -1;

    for (unsigned int i=0;i<buttons.size();i++)
	if (buttons[i].sendOSC() < 0)
	    return -1;

    return 0;
}


void Settings::sendOSC(int selectedGroup) const {
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
	dbg("Setting.sendOSC",3) << "Sending " << path << "," << label << std::endl;
	if (TouchOSC::instance()->send((path+"/label"),label) <0 ) 
	    return;
    }

    // Send settings for current select
    const Setting *s= getSetting(selectedGroup);
    if (s==NULL) {
	dbg("TouchOSC.sendOSC",1) << "Selected group " << selectedGroup << " is not valid, switching to group 0" << std::endl;
	s=getSetting(0);
	if (s==NULL)
	    return;
    }
    if(s->sendOSC() <0)
	return;
}

void TouchOSC::sendOSC() const {
    // Send OSC to make UI reflect current values
    dbg("TouchOSC.sendOSC",1) << "Sending OSC updates with group " << selectedGroup << " to " << remote << std::endl;
    settings.sendOSC(selectedGroup);
    if (send("/ui/freeze",frozen?1.0:0.0) < 0) return;
    if (send("/ui/body",bodyEnabled?1.0:0.0) < 0) return;
    if (send("/ui/legs",legsEnabled?1.0:0.0) < 0) return;
    if (send("/ui/attrenable",attrsEnabled?1.0:0.0) < 0) return;
    if (send("/ui/layer",layeringEnabled?1.0:0.0) < 0) return;
    if (send("/ui/oneper",onePerEnabled?1.0:0.0) < 0) return;
    if (send("/ui/fusion",fusionEnabled?1.0:0.0) < 0) return;
    if (send("/ui/maxconn/label","Max Connections: "+std::to_string(maxConnections))<0) return;
    if (send("/ui/visthresh/label","Visual Threshold: "+std::to_string(round(visualThreshold*100)/100))<0) return;
    if (send("/ui/condglobal/label","Conductor Global: "+std::to_string(round(conductorGlobal*100/100)))<0) return;
}

void TouchOSC::frameTick_impl(int frame) {
    dbg("TouchOSC.frameTick",3) << "Frame " << frame << std::endl;
    if (frame % 10 == 0) {
	activityLED=!activityLED;
	if (send("/ui/active1",activityLED?1.0f:0.0f) <0 ) 
	    return;
	if (send("/ui/active2",activityLED?0.0f:1.0f) <0 ) 
	    return;
    }
    if (frame % 50 == 4) {
	updateConnectionMap();
    }
    if (frame % 10 == 2) {
	sendOSC();
    }
}

Fader *TouchOSC::getFader_impl(std::string groupName, std::string faderName) {
    bool updatedUI=false;
    Setting *s=settings.getSetting(groupName);
    if (s==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested group " << groupName << " does not exists; adding." << std::endl;
	settings.addGroup(groupName);
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

Button *TouchOSC::getButton_impl(std::string groupName, std::string buttonName) {
    bool updatedUI=false;
    Setting *s=settings.getSetting(groupName);
    if (s==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested group " << groupName << " does not exists; adding." << std::endl;
	settings.addGroup(groupName);
	s=settings.getSetting(groupName);
	updatedUI=true;
    }
    Button *f=s->getButton(buttonName);
    if (f==NULL) {
	dbg("TouchOSC.getValue",1) << "Requested button " << buttonName << " does not exist in group " << groupName << " adding it" << std::endl;
	s->addButton(buttonName);
	f=s->getButton(buttonName);
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
		dbg("TouchOSC.conn",1) << "Selected group position " << col << "/" << row << " with value " << argv[0]->f << std::endl;
		if (argv[0]->f>0.5) {
		    int gpos=row*4+col;
		    const Setting *newGroup=settings.getSetting(gpos);
		    if (newGroup==NULL) {
			dbg("TouchOSC.conn",1) << "No group at position " << gpos << std::endl;
		    } else {
			selectedGroup=gpos;
			dbg("TouchOSC.conn",1) << "Switch to group " << newGroup->getGroupName() << std::endl;
			sendOSC();
		    }
		}
		handled=true;
	    } else if (strcmp(tok,"faders")==0) {
		int pos=atoi(strtok(NULL,"/"))-1;
		dbg("TouchOSC.faders",1) << "Set fader " << pos << " with value " << argv[0]->f << std::endl;
		Setting *s=settings.getSetting(selectedGroup);
		if (s==NULL) {
		    dbg("TouchOSC.faders",1) << "Selected group " << selectedGroup << " is not valid." << std::endl;
		} else {
		    Fader *f=s->getFader(pos);
		    if (f==NULL) {
			dbg("TouchOSC.faders",1) << "Selected fader " << pos << " is not valid." << std::endl;
		    } else {
			f->set(argv[0]->f);
		    }
		}
		handled=true;
	    } else if (strcmp(tok,"toggles")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("TouchOSC.toggles",1) << "Set toggle " << col << "/" << row << " with value " << argv[0]->f << std::endl;
		Setting *s=settings.getSetting(selectedGroup);
		if (s==NULL) {
		    dbg("TouchOSC.toggles",1) << "Selected group " << selectedGroup << " is not valid." << std::endl;
		} else {
		    Fader *f=s->getFader(col);
		    if (f==NULL) {
			dbg("TouchOSC.toggles",1) << "Selected fader " << col << " is not valid." << std::endl;
		    } else {
			if (row==0)
			    f->setUseValueToggle(argv[0]->f>0.5);
			else
			    dbg("TouchOSC.toggles",1) << "Bad row: " << row << " from /ui/conn/toggles" << std::endl;
		    }
		}
		handled=true;
	    } else if (strcmp(tok,"buttons")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		int row=atoi(strtok(NULL,"/"))-1;
		int pos=col+row*Setting::MAXBUTTONS;
		dbg("TouchOSC.buttons",1) << "Received command to set button " << pos << " with value " << argv[0]->f << std::endl;
		Setting *s=settings.getSetting(selectedGroup);
		if (s==NULL) {
		    dbg("TouchOSC.buttons",1) << "Selected group " << selectedGroup << " is not valid." << std::endl;
		} else {
		    Button *f=s->getButton(pos);
		    if (f==NULL) {
			dbg("TouchOSC.buttons",1) << "Selected button " << pos << " is not valid in group " << s->getGroupName() << std::endl;
		    } else {
			f->set(argv[0]->f>0.5);
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
		    dbg("TouchOSC.preset",1) << "Missed preset " << tok << " press" << std::endl;
		} else {
		    float delta=(now.tv_sec-pressTime.tv_sec)+(now.tv_usec-pressTime.tv_usec)/1e6;
		    dbg("TouchOSC.preset",1) << "Got preset " << tok << " held for " << delta << " seconds" << std::endl;
		    if (strcmp(tok,"10")) 
			// Defaults
			load(std::string("settings_default.txt"));

		    if (delta>1) 
			save(std::string("settings_")+tok+".txt");
		    else
			load(std::string("settings_")+tok+".txt");
		    pressTime.tv_sec=0;
		}
	    }
	    handled=true;
	} else if (strcmp(tok,"linked") == 0) {
	    int col=atoi(strtok(NULL,"/"))-1;
	    int row=atoi(strtok(NULL,"/"))-1;
	    dbg("TouchOSC.linked",1) << "Got linked touch  at " << col << "," << row << " v=" << argv[0]->f << std::endl;
	    std::vector<int> uids = People::instance()->getIDs();
	    if (col>=uids.size() || row>=uids.size()) {
		dbg("TouchOSC.linked",1) << "Only have " << uids.size() << "UIDs; touch out of range" << std::endl;
	    } else {
		trackUID1=uids[row];
		trackUID2=uids[col];
		dbg("TouchOSC.linked",1) << "Displaying UIDs " << trackUID1 << "," << trackUID2 << std::endl;
	    }
	    struct timeval now;
	    gettimeofday(&now,0);
	    if (argv[0]->f > 0.5)
		pressTime=now;
	    else {
		if (pressTime.tv_sec==0) {
		    dbg("TouchOSC.linked",1) << "Missed linked " << tok << " press" << std::endl;
		} else {
		    float delta=(now.tv_sec-pressTime.tv_sec)+(now.tv_usec-pressTime.tv_usec)/1e6;
		    dbg("TouchOSC.linked",1) << "Got linked held for " << delta << " seconds" << std::endl;
		    if (delta>1) {
			// Force a connection or attribute between the selected people
			Setting *s=settings.getSetting(selectedGroup);
			if (s==NULL) {
			    dbg("TouchOSC.linked",1) << "No attribute selected" << std::endl;
			} else {
			    std::string attribute = s->getGroupName();
			    if (trackUID1!= trackUID2) {
				// A connection
				CIDType cid;
				if (trackUID1>trackUID2) 
				    std::swap(trackUID1,trackUID2);
				Connection conn(std::to_string(trackUID1)+"-"+std::to_string(trackUID2),trackUID1,trackUID2);
				conn.set("persistent",attribute,trackUID1,trackUID2);
				conn.setAge(-100);
				dbg("TouchOSC.linked",1) << "Adding connection " << conn << std::endl;
				Connections::instance()->add(conn);
			    }
			}
		    }
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
	} else if (strcmp(tok,"attrenable")==0) {
	    attrsEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"layer")==0) {
	    layeringEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"oneper")==0) {
	    onePerEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"fusion")==0) {
	    fusionEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"freeze")==0) {
	    dbg("TouchOSC.freeze",1) << "Got message: " << argv[0]->f << std::endl;
	    frozen=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"maxconn")==0) {
	    maxConnections=argv[0]->f;
	    handled=true;
	} else if (strcmp(tok,"visthresh")==0) {
	    visualThreshold=argv[0]->f;
	    handled=true;
	} else if (strcmp(tok,"condglobal")==0) {
	    // Pass on to conductor TODO
	    Conductor::instance()->send("/ui/condglobal",argv[0]->f);
	    conductorGlobal=argv[0]->f;
	    handled=true;
	}	    
    }
    if (handled) {
	sendOSC();
    } else {
	dbg("TouchOSC.handleOSCMessage",1) << "Unhanded message: " << path << ": parse failed at token: " << tok << std::endl;
    }
    
    delete [] pathCopy;
    return handled?0:1;
}

void TouchOSC::save(std::string filename) const {
    try {
	std::ofstream ofs(filename);
	if (!ofs.good()) {
	    std::cerr << "Failed open of " << filename << " for saving" << std::endl;
	    return;
	}
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
	if (!ifs.good()) {
	    std::cerr << "Failed open of " << filename << " for loading" << std::endl;
	    return;
	}
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
    dbg("TouchOSC.updateConnectionMap",1) << "Updating connection map" << std::endl;
    static const int MAXUIDDISPLAY=10;
    std::vector<int> uids = People::instance()->getIDs();
    for (int i=0;i<MAXUIDDISPLAY;i++) {
	std::string uidstring = "";
	if (i<uids.size())
	    uidstring = std::to_string(uids[i]);
	if (send("/ui/uid/"+std::to_string(i+1),uidstring) <0 ) 
	    return;
	for (int j=0;j<MAXUIDDISPLAY;j++) {
	    bool connected=false;
	    if (i<uids.size() && j<uids.size())
		connected = Connections::instance()->isConnected(uids[i],uids[j]) || Connections::instance()->isConnected(uids[j],uids[i]);
	    std::string path="/ui/linked/"+std::to_string(j+1)+"/"+std::to_string(i+1);
	    dbg("TouchOSC.updateConnectionMap",5) << "Send " << path << "," << (connected?1.0f:0.0f) << std::endl;

	    if (send(path,connected?1.0f:0.0f) <0 )
		return;
	}
    }
    
    // Display attributes of selected UIDs
    std::string uid1label="";
    std::string uid2label="";
    if (trackUID1 >= 0 && People::instance()->personExists(trackUID1)) {
	uid1label=std::to_string(trackUID1);
	Attributes attr=People::instance()->getAttributes(trackUID1);
	std::vector<std::string> attrNames=attr.getAttributeNames();
	for (int i=0;i<attrNames.size();i++) {
	    Attribute a=attr.get(attrNames[i]);
	    std::string msg=uid1label+": "+attrNames[i]+"="+std::to_string(a.getValue());
	    std::string path="/ui/connattr/"+std::to_string(i+1);
	    if (send(path,msg) <0 )
		return;
	}
	int row=attrNames.size();
	// And connections with other ID
	if (trackUID2 >= 0 && People::instance()->personExists(trackUID2)) {
	    uid2label=std::to_string(trackUID2);
	    if (Connections::instance()->isConnected(trackUID1,trackUID2)) {
		Connection c=Connections::instance()->getConnection(trackUID1,trackUID2);
		Attributes attr=c.getAttributes();
		std::vector<std::string> attrNames=attr.getAttributeNames();
		for (int i=0;i<attrNames.size();i++) {
		    Attribute a=attr.get(attrNames[i]);
		    std::string msg=uid1label+"-"+uid2label+": "+attrNames[i]+"="+std::to_string(a.getValue());
		    std::string path="/ui/connattr/"+std::to_string(i+row+1);
		    if (send(path,msg) <0 )
			return;
		}
		row+=attrNames.size();
	    }
	}
	// Blank out remaining attribute names
	static const int MAXATTR=10;
	for (int i=row;i<MAXATTR;i++) {
	    std::string path="/ui/connattr/"+std::to_string(i+1);
	    if (send(path,"") <0 ) {
		return;
	    }
	}
    }
    if (send("/ui/connattr/uid1",uid1label) <0 ) 
	return;
    if (send("/ui/connattr/uid2",uid2label) <0 )
	return;
}

int TouchOSC::send(std::string path, float value) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << value << std::endl;
    if (lo_send(remote,path.c_str(),"f",value) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

int TouchOSC::send(std::string path, std::string value) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << value << std::endl;
    if (lo_send(remote,path.c_str(),"s",value.c_str()) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << lo_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}
