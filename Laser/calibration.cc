#include "calibration.h"
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

Calibration::Calibration(int nunits): relMappings(NUMRELATIVE), absMappings(NUMABSOLUTE)  {
    dbg("Calibration.Calibration",1) << "Constructing calibration with " << nunits << " units." << std::endl;
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]=std::shared_ptr<RelMapping>(new RelMapping(i,nunits));
    for (int i=0;i<absMappings.size();i++)
	absMappings[i]=std::shared_ptr<AbsMapping>(new AbsMapping(i,nunits));
    flipX=false;
    flipY=false;
    speed=100;
    absSelected=-1;
    relSelected=-1;
    updateUI();
    showStatus("initalized");
}

int Calibration::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Calibration.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;

    std::string host=lo_address_get_hostname(lo_message_get_source(msg));
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"cal")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"rel")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"sel")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("Calibration",1) << "Selected relative position " << row << " with value " << argv[0]->f <<  std::endl;
		if (argv[0]->f > 0)
		    relSelected=row;
		else if (argv[0]->f == 0 && relSelected==row)
		    relSelected=-1;
		handled=true;
	    }
	}
    }
    updateUI();
    return handled?0:1;
}

void Calibration::updateUI() const {
    if (relSelected>=0) {
	relMappings[relSelected]->updateUI();
	send("/cal/rel/sel/1/"+std::to_string(relSelected+1),1.0);
    } else if (absSelected>=0) {
	absMappings[absSelected]->updateUI();
	send("/cal/abs/sel/1/"+std::to_string(absSelected+1),1.0);
    } else
	dbg("Calibration.updateUI",1) << "Nothing selected" << std::endl;
    send("/cal/flipx",flipX?1.0:0.0);
    send("/cal/flipy",flipY?1.0:0.0);
    send("/cal/speed",speed);
}

void RelMapping::updateUI() const {
    for (int i=0;i<locked.size();i++) {
	send("/cal/lock/"+std::to_string(i)+"/1",locked[i]?1.0:0.0);
	send("/cal/adjust/"+std::to_string(i)+"/1",(i==adjust)?1.0:0.0);
	if (adjust<0) {
	    send("/cal/dev/x","");
	    send("/cal/dev/y","");
	} else {
	    send("/cal/dev/x",std::to_string(devpt[adjust].X()));
	    send("/cal/dev/y",std::to_string(devpt[adjust].Y()));
	    send("/cal/dev/xy",devpt[adjust].X(),devpt[adjust].Y());
	}
    }
}

void AbsMapping::updateUI() const {
    RelMapping::updateUI();
    send("/cal/abs/lock/"+std::to_string(id+1)+"/1",locked?1.0:0.0);
    send("/cal/abs/"+std::to_string(id+1)+"/x",floor.X());
    send("/cal/abs/"+std::to_string(id+1)+"/y",floor.Y());
}

// Display status in touchOSC
void Calibration::showStatus(std::string line1,std::string line2, std::string line3) {
    send("/cal/status/1",line1);
    send("/cal/status/2",line2);
    send("/cal/status/3",line3);
}
