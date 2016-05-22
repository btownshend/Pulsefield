#include <fstream>
#include <sys/time.h>
#include "touchosc.h"

TouchOSC *TouchOSC::theInstance=NULL;
static const std::string flags[]={"body","legs","grid","background","alignment","test","outline","allocationTest","intensity"};

TouchOSC::TouchOSC(URLConfig &urls)  {
    theInstance=this;

    /* Setup touchOSC sending */
    int touchOSCPort=urls.getPort("TO");
    const char *touchOSCHost=urls.getHost("TO");
    if (touchOSCPort==-1 || touchOSCHost==0) {
	fprintf(stderr,"Unable to locate TO in urlconfig.txt\n");
	remote=0;
    } else {
	char cbuf[10];
	sprintf(cbuf,"%d",touchOSCPort);
	remote = lo_address_new(touchOSCHost, cbuf);
	dbg("TouchOSC",1)  << "Set remote to " << loutil_address_get_url(remote) << std::endl;
	// Don't call sendOSC here as it will cause a recursive loop
    }

    // Setup 
    selectedGroup=0;
    selectedLaser=-1;
    frozen=false;
    layeringEnabled=false;
    onePerEnabled=true;
    attrsEnabled=true;
    fusionEnabled=true;
    labelsEnabled=true;
    maxConnections=10;
    visualThreshold=0.0;
    conductorGlobal=1.0;
    cellGlobal=1.0;
    pressTime.tv_sec=0;
    trackUID1=-1;
    trackUID2=-1;
}
		       
TouchOSC::~TouchOSC() {
    if (remote)
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

void TouchOSC::frameTick_impl(int frame) {
    dbg("TouchOSC.frameTick",3) << "Frame " << frame << std::endl;
    if (frame % 50 == 0) {
	static bool toggle;
	if (send("/health/LS",toggle?1.0f:0.0f) <0 ) 
	    return;
	toggle=!toggle;
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
    return f;
}

int TouchOSC::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("TouchOSC.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;

    std::string host=lo_address_get_hostname(lo_message_get_source(msg));
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
		    if (strcmp(tok,"10")==0) 
			// Defaults
			load(std::string("settings_default.txt"));
		    else if (delta>1) 
			save(std::string("settings_")+tok+".txt");
		    else
			load(std::string("settings_")+tok+".txt");
		    pressTime.tv_sec=0;
		}
	    }
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
	} else if (strcmp(tok,"labels")==0) {
	    labelsEnabled=argv[0]->f>0.5;
	    handled=true;
	} else if (strcmp(tok,"maxconn")==0) {
	    maxConnections=argv[0]->f;
	    handled=true;
	} else if (strcmp(tok,"visthresh")==0) {
	    visualThreshold=argv[0]->f;
	    handled=true;
	}	    
    }
    if (handled) {
	;
    } else {
	dbg("TouchOSC.handleOSCMessage",1) << "Unhandled message: " << path << ": parse failed at token: " << tok << std::endl;
    }
    
    delete [] pathCopy;
    return handled?0:1;
}


void TouchOSC::save(std::string filename) const {
    try {
	dbg("TouchOSC.save",1) << "Saving settings in " << filename << std::endl;
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
    } catch (std::ifstream::failure e) {
	std::cerr << "Exception loading from " << filename << ": " << e.what() << std::endl;
    } catch (boost::archive::archive_exception e) {
	std::cerr << "Exception loading from " << filename << ": " << e.what() << std::endl;
    }
}


int TouchOSC::send(std::string path, float value) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << value << std::endl;
    if (lo_send(remote,path.c_str(),"f",value) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

int TouchOSC::send(std::string path, float v1, float v2) const {
dbg("TouchOSC.send",4) << "send " << path << "," << v1 << ", " << v2 << std::endl;
    if (lo_send(remote,path.c_str(),"ff",v1,v2) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

#if 0
int TouchOSC::send(std::string path, float v1, float v2, float v3, float v4) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << v1 << ", " << v2 << ", " << v3 << ", " << v4 << std::endl;
    if (lo_send(remote,path.c_str(),"ffff",v1,v2,v3,v4) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

int TouchOSC::send(std::string path, float v1, float v2, float v3, float v4, float v5, float v6, float v7) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << v1 << ", " << v2 << ", " << v3 << ", " << v4 << "..." << std::endl;
    if (lo_send(remote,path.c_str(),"fffffff",v1,v2,v3,v4,v5,v6,v7) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

int TouchOSC::send(std::string path, float v1, float v2, float v3, float v4, float v5, float v6, float v7,float v8, float v9, float v10) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << v1 << ", " << v2 << ", " << v3 << ", " << v4 << "..." << std::endl;
    if (lo_send(remote,path.c_str(),"ffffffffff",v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}

int TouchOSC::send(std::string path, float v1, float v2, float v3, float v4, float v5, float v6, float v7,float v8, float v9, float v10,float v11, float v12, float v13, float v14, float v15, float v16) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << v1 << ", " << v2 << ", " << v3 << ", " << v4 << "..." << std::endl;
    if (lo_send(remote,path.c_str(),"ffffffffff",v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}
#endif

int TouchOSC::send(std::string path, std::string value) const {
    dbg("TouchOSC.send",4) << "send " << path << "," << value << std::endl;
    if (lo_send(remote,path.c_str(),"s",value.c_str()) <0 ) {
	dbg("TouchOSC.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(10);
    return 0;
}
