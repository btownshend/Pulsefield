#include <set>
#include "frontend.h"
#include "sickio.h"
#include "parameters.h"

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message , void *) {
    static std::set<std::string> noted;  // Already noted
    if (noted.count(std::string(path)+types) == 0) {
	noted.insert(std::string(path)+types);
	int i;
	fprintf(stdout, "Unhandled Message Rcvd: %s (", path);
	for (i=0; i<argc; i++) {
	    fprintf(stdout,"%c",types[i]);
	}
	fprintf(stdout, "): ");
	for (i=0; i<argc; i++) {
	    lo_arg_pp((lo_type)types[i], argv[i]); // Pretty-prints to stdout
	    fprintf(stdout, ", ");
	}
	fprintf(stdout,"\n");
	fflush(stdout);
    }
    return 1;
}

bool FrontEnd::doQuit = false;

static int quit_handler(const char *, const char *, lo_arg **, int, lo_message , void *user_data) { ((FrontEnd *)user_data)->quit(); return 0; }
void FrontEnd::quit() {
    printf("Received /quit command, quitting\n");
    doQuit = true;
}

static int start_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(true); return 0; }
static int stop_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(false); return 0; }
void FrontEnd::startStop(bool start) {
	printf("FrontEnd: %s\n", start?"start":"stop");
	if (start) {
	    for (int i=0;i<nsick;i++)
		if (sick[i]->start() < 0 ) {
			fprintf(stderr,"Failed to start sensor %d, aborting startup\n", sick[i]->getId());
			start=false;
			// Stop any cameras we already started
			for (int j=0;j<=i;j++)
			    (void)sick[j]->stop();
			break;
		}
	} else  {
	    for (int i=0;i<nsick;i++)
		(void)sick[i]->stop();
	}

	// Send status update message
	for (int i=0;i<dests.size();i++) {
		char cbuf[10];
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		if (start)
			lo_send(addr,"/vis/started","");
		else
			lo_send(addr,"/vis/stopped","");
		lo_address_free(addr);
	}
}


static int setFPS_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setFPS(argv[0]->i); return 0; }
void FrontEnd::setFPS(int fps) {
    dbg("FrontEnd.setFPS",1) << "Setting all sensors to " << fps << " FPS" << std::endl;
	for (int i=0;i<nsick;i++)
		sick[i]->setScanFreq(fps);
}

int FrontEnd::getFPS() const {
    if (nsick>0)
	return sick[0]->getScanFreq();
    //fprintf(stderr,"Warning: assuming recorded file is at 50 FPS\n");
    return 50;
}

static int setRes_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setRes(argv[0]->i,&argv[1]->s); return 0; }
void FrontEnd::setRes(int sickid, const char *res) {
    dbg("FrontEnd.setRes",1) << "Setting sensors " << sickid << " to resolution " << res  << " degrees." << std::endl;
	if (sickid<0 || sickid>=nsick) {
		fprintf(stderr,"setRes: bad sensor number: %d\n", sickid);
		return;
	}
	double dres=atof(res);
	sick[sickid]->setScanRes(dres);
}

static int setEchoes_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setEchoes(argv[0]->i); return 0; }
void FrontEnd::setEchoes(int echoes) {
	if (echoes<1 || echoes>SickIO::MAXECHOES) {
		fprintf(stderr,"setEchoes: bad number of echoes: %d\n",echoes);
	}
	dbg("FrontEnd.setEchoes",1) << "Setting all sensors to return " << echoes << " echoes." << std::endl;
	nechoes=echoes;
}


static int getRange_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::RANGE,argv[0]->i); return 0; }
static int getReflect_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::REFLECT,argv[0]->i); return 0; }
void FrontEnd::getStat(long int stat, int mode) {
    dbg("FrontEnd.getStat",1) << "getStat(" << stat << "," << mode << ")" << std::endl;
	if (mode==2)
		sendAlways |= stat;
	else
		sendAlways &= ~stat;

	if (mode>0)
		sendOnce |= stat;
}

static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
void FrontEnd::addDest(const char *host, int port) {
	dests.add(host,port);
}

static int addDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(msg,argv[0]->i); return 0; }
void FrontEnd::addDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	addDest(host,port);
	sendSetupMessages(host,port);
}

static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
void FrontEnd::rmDest(const char *host, int port) {
	dests.remove(host,port);
}

static int rmDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(msg,argv[0]->i); return 0; }
void FrontEnd::rmDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	rmDest(host,port);
}

static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmAllDest(); return 0; }
void FrontEnd::rmAllDest() {
	dests.removeAll();
}

static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->ping(msg,argv[0]->i); return 0; }
void FrontEnd::ping(lo_message msg, int seqnum) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	printf("Got ping from %s\n",host);
	for (int i=0;i<dests.size();i++) {
		char cbuf[10];
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		lo_send(addr,"/ack","i",seqnum);
		lo_address_free(addr);
	}
}

static int maxrange_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    MAXRANGE=argv[0]->f*UNITSPERM; return 0; }

void FrontEnd::addHandlers() {
	/* add method that will match any path and args if they haven't been caught explicitly */
	lo_server_add_method(s, NULL, NULL, generic_handler, NULL);

	lo_server_add_method(s, "/quit", "", quit_handler, this);

	lo_server_add_method(s,"/vis/start","",start_handler,this);
	lo_server_add_method(s,"/vis/stop","",stop_handler,this);

	lo_server_add_method(s,"/vis/set/fps","i",setFPS_handler,this);
	lo_server_add_method(s,"/vis/set/res","is",setRes_handler,this);
	lo_server_add_method(s,"/vis/set/echoes","i",setEchoes_handler,this);
	lo_server_add_method(s,"/ping","i",ping_handler,this);

	lo_server_add_method(s,"/vis/get/range","i",getRange_handler,this);
	lo_server_add_method(s,"/vis/get/reflect","i",getReflect_handler,this);

	lo_server_add_method(s,"/vis/dest/add","si",addDest_handler,this);
	lo_server_add_method(s,"/vis/dest/add/port","i",addDestPort_handler,this);
	lo_server_add_method(s,"/vis/dest/remove","si",rmDest_handler,this);
	lo_server_add_method(s,"/vis/dest/remove/port","i",rmDestPort_handler,this);
	lo_server_add_method(s,"/vis/dest/clear","",rmAllDest_handler,this);

	lo_server_add_method(s,"/pf/maxrange","f",maxrange_handler,this);
}
