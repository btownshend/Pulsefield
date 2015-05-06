#include "calibration.h"
#include "lasers.h"
#include "dbg.h"

static const int NUMRELATIVE=10;
static const int NUMABSOLUTE=4;

std::shared_ptr<Calibration> Calibration::theInstance;   // Singleton

static void send(std::string path, float value)  {
    TouchOSC::instance()->send(path,value);
}
static void send(std::string path, std::string value)  {
    TouchOSC::instance()->send(path,value);
}
static void send(std::string path, float val1, float val2)  {
    TouchOSC::instance()->send(path,val1,val2);
}

bool RelMapping::handleOSCMessage(char *tok, lo_arg **argv,int speed) {
    dbg("RelMapping",1) << "tok=" << tok << ", speed=" << speed << ", adjust=" << adjust << ", id=" << id << ", nunits=" << locked.size() <<  std::endl;
    bool handled=false;
    if (strcmp(tok,"adjust")==0) {
	int col=atoi(strtok(NULL,"/"))-1;
	//int row=atoi(strtok(NULL,"/"))-1;
	if (argv[0]->f>0)
	    adjust=col;
	else if (adjust==col)
	    adjust=-1;
	if (adjust>=devpt.size())
	    adjust=-1;
	handled=true;
    } else if (strcmp(tok,"dev")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"xy")==0) {
	    // xy pad
	    if (adjust>=0 && ~locked[adjust]) {
		devpt[adjust].setX(argv[0]->f);
		devpt[adjust].setY(argv[1]->f);
	    }
	    handled=true;
	}
    } else if (strcmp(tok,"lock")==0) {
	int col=atoi(strtok(NULL,"/"))-1;
	//	int row=atoi(strtok(NULL,"/"))-1;
	dbg("RelMapping",1) << "lock: col=" << col << ", size(locked)=" << locked.size() << std::endl;
	if (col<locked.size())
	    locked[col]=argv[0]->f > 0;
	handled=true;
    } else if (strcmp(tok,"left")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setX(std::max(-32767.0f,devpt[adjust].X()-speed));
	handled=true;
    } else if (strcmp(tok,"right")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setX(std::min(32767.0f,devpt[adjust].X()+speed));
	handled=true;
    } else if (strcmp(tok,"up")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setY(std::min(32767.0f,devpt[adjust].Y()+speed));
	handled=true;
    } else if (strcmp(tok,"down")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setY(std::max(-32767.0f,devpt[adjust].Y()-speed));
	handled=true;
    } else if (strcmp(tok,"center")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust]=Point(0,0);
	handled=true;
    }
    
    return handled;
}


Calibration::Calibration(int _nunits): relMappings(NUMRELATIVE), absMappings(NUMABSOLUTE)  {
    nunits = _nunits;
    dbg("Calibration.Calibration",1) << "Constructing calibration with " << nunits << " units." << std::endl;
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]=std::shared_ptr<RelMapping>(new RelMapping(i,nunits));
    for (int i=0;i<absMappings.size();i++)
	absMappings[i]=std::shared_ptr<AbsMapping>(new AbsMapping(i,nunits));
    flipX=false;
    flipY=false;
    speed=100;
    absSelected=-1;
    relSelected=0;
    updateUI();
    showStatus("Initalized");
}

int Calibration::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Calibration.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;

    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"cal")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"rel")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"sel")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		assert(col==0);
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("Calibration",1) << "Selected relative position " << row << " with value " << argv[0]->f <<  std::endl;
		if (argv[0]->f > 0) {
		    relSelected=row;
		    if (absSelected>=0) {
			send("/cal/abs/sel/1/"+std::to_string(absSelected+1),0.0);
			absSelected=-1;
		    }
		    showStatus("Adjusting relative anchor point "+std::to_string(row));
		}
		handled=true;
	    }
	} else if (strcmp(tok,"abs")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"sel")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		assert(col==0);
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("Calibration",1) << "Selected absolute position " << row << " with value " << argv[0]->f <<  std::endl;
		if (argv[0]->f > 0) {
		    absSelected=row;
		    if (relSelected>=0) {
			send("/cal/rel/sel/1/"+std::to_string(relSelected+1),0.0);
			relSelected=-1;
		    }
		    showStatus("Adjusting absolute anchor point "+std::to_string(row));
		}
		handled=true;
	    }
	} else if (strcmp(tok,"speed")==0) {
	    speed=argv[0]->f;
	    showStatus("Set speed to " + std::to_string(std::round(speed)));
	    handled=true;
	}  else if (strcmp(tok,"flipx")==0) {
	    flipX=argv[0]->f>0;
	    handled=true;
	}  else if (strcmp(tok,"flipy")==0) {
	    flipY=argv[0]->f>0;
	    handled=true;
	} else if (strcmp(tok,"save")==0) {
	    Lasers::instance()->save();
	    handled=true;
	} else if (strcmp(tok,"load")==0) {
	    Lasers::instance()->load();
	    handled=true;
	}  else if (relSelected>=0) {
	    handled=relMappings[relSelected]->handleOSCMessage(tok,argv,speed);
	} else if (absSelected>=0) {
	    handled=absMappings[absSelected]->handleOSCMessage(tok,argv,speed);
	}
    }
    updateUI();
    delete pathCopy;
    return handled?0:1;
}

void Calibration::updateUI() const {
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]->updateUI(i==relSelected);
    for (int i=0;i<absMappings.size();i++)
	absMappings[i]->updateUI(i==absSelected);

    send("/cal/flipx",flipX?1.0:0.0);
    send("/cal/flipy",flipY?1.0:0.0);
    send("/cal/speed",speed);
}

void RelMapping::updateUI(bool selected) const {
    if (selected) {
	if (id>=0)
	    send("/cal/rel/sel/1/"+std::to_string(id+1),1.0);
	for (int i=0;i<locked.size();i++) {
	    send("/cal/lock/"+std::to_string(i+1)+"/1",locked[i]?1.0:0.0);
	    send("/cal/status/"+std::to_string(i+1)+"/"+std::to_string(id+1),locked[i]?1.0:0.0);
	    send("/cal/adjust/"+std::to_string(i+1)+"/1",(i==adjust)?1.0:0.0);
	    if (adjust<0) {
		send("/cal/dev/x","");
		send("/cal/dev/y","");
	    } else {
		send("/cal/dev/x",std::to_string((int)std::round(devpt[adjust].X())));
		send("/cal/dev/y",std::to_string((int)std::round(devpt[adjust].Y()+0.5)));
		send("/cal/dev/xy",devpt[adjust].X(),devpt[adjust].Y());
	    }
	}
    }
}

void AbsMapping::updateUI(bool selected) const {
    // Retrieive fiducial location
    ((AbsMapping *)this)->floor=Lasers::instance()->getLaser(0)->getTransform().getFloorPoint(id);
    RelMapping::updateUI(selected);
    if (selected)
	send("/cal/abs/sel/1/"+std::to_string(id+1),1.0);
    send("/cal/abs/lock/"+std::to_string(id+1)+"/1",glocked?1.0:0.0);
    send("/cal/abs/"+std::to_string(id+1)+"/x",std::round(floor.X()*100)/100);
    send("/cal/abs/"+std::to_string(id+1)+"/y",std::round(floor.Y()*100)/100);
}

// Display status in touchOSC
void Calibration::showStatus(std::string line1,std::string line2, std::string line3) const {
    send("/cal/status/1",line1);
    send("/cal/status/2",line2);
    send("/cal/status/3",line3);
}

void AbsMapping::save(ptree &p) const {
    p.put("id",id);
    p.put("floor.x",floor.X());
    p.put("floor.y",floor.Y());
    RelMapping::save(p);
}

void AbsMapping::load(ptree &p)  {
    id=p.get("id",id);
    floor=Point(p.get("floor.x",floor.X()),p.get("floor.y",floor.Y()));
    RelMapping::load(p);
}

void RelMapping::save(ptree &p) const {
    dbg("RelMapping.save",1) << "Saving relMapping " << id << " to ptree" << std::endl;
    if (id>=0)
	p.put("id",id);
    p.put("adjust",adjust);
    ptree dpts;
    for (int i=0;i<devpt.size();i++) {
	ptree dp;
	dp.put("x",devpt[i].X());
	dp.put("y",devpt[i].Y());
	dp.put("locked",locked[i]);
	dpts.push_back(std::make_pair("",dp));
    }
    p.put_child("devpts",dpts);
}

void RelMapping::load(ptree &p) {
    dbg("RelMapping.save",1) << "Loading relMapping from ptree" << std::endl;
    id=p.get("id",-1);
    adjust=p.get("adjust",adjust);

    try {
	ptree dp=p.get_child("devpts");
	int i=0;
	for (ptree::iterator v = dp.begin(); v != dp.end();++v) {
	    if (i>=devpt.size())
		break;
	    ptree val=v->second;
	    devpt[i]=Point(val.get<double>("x",devpt[i].X()),val.get<double>("y",devpt[i].Y()));
	    locked[i]=val.get<bool>("locked",locked[i]);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'devpts' in laser settings" << std::endl;
    }
}

void Calibration::save(ptree &p) const {
    dbg("Calibration.save",1) << "Saving calibration to ptree" << std::endl;
    ptree rm;
    for (unsigned int i=0;i<relMappings.size();i++)  {
	ptree mapping;
	relMappings[i]->save(mapping);
	rm.push_back(std::make_pair("",mapping));
    }
    p.put_child("rel",rm);
    ptree am;
    for (unsigned int i=0;i<absMappings.size();i++)  {
	ptree mapping;
	absMappings[i]->save(mapping);
	am.push_back(std::make_pair("",mapping));
    }
    p.put_child("abs",am);
    p.put("speed",speed);
    p.put("flipX",flipX);
    p.put("flipY",flipY);
    showStatus("Saved configuration");
}

void Calibration::load(ptree &p) {
    dbg("Calibration.load",1) << "Loading transform from ptree" << std::endl;
    speed=p.get("speed",speed);
    flipX=p.get("flipX",flipX);
    flipY=p.get("flipY",flipY);
    ptree rm;
    try {
	rm=p.get_child("rel");
	int i=0;
	for (ptree::iterator v = rm.begin(); v != rm.end();++v) {
	    if (i>=relMappings.size())
		break;
	    relMappings[i]->load(v->second);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'rel' in laser settings" << std::endl;
    }
    ptree am;
    try {
	am=p.get_child("abs");
	int i=0;
	for (ptree::iterator v = am.begin(); v != am.end();++v) {
	    if (i>=absMappings.size())
		break;
	    absMappings[i]->load(v->second);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'abs' in laser settings" << std::endl;
    }
    showStatus("Loaded configuration");
}

