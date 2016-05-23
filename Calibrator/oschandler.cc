#include <iomanip>
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <dbg.h>
#include "oschandler.h"
#include "calibration.h"

static void error(int num, const char *msg, const char *path)
{
	fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message msg , void *user_data) {
    dbg("generic_handler",5) << "Received message: " << path << ", with types: " << types << std::endl;
    int nothandled=1;
    if (strncmp(path,"/cal/",5)==0) {
	// Calibration messages
	nothandled=Calibration::handleOSCMessage(path,types,argv,argc,msg);
    }
    if (nothandled) {
	static std::set<std::string> noted;  // Already noted
	if (noted.count(std::string(path)+types) == 0) {
	    noted.insert(std::string(path)+types);
	    int i;
	    printf( "Unhandled Message Rcvd: %s (", path);
	    for (i=0; i<argc; i++) {
		printf("%c",types[i]);
	    }
	    printf( "): ");
	    for (i=0; i<argc; i++) {
		lo_arg_pp((lo_type)types[i], argv[i]);
		printf( ", ");
	    }
	    printf("\n");
	    fflush(stdout);
	}
    }
    return nothandled;
}

static bool doQuit = false;

static int quit_handler(const char *, const char *, lo_arg **, int, lo_message , void *) {
	printf("Received /quit command, quitting\n");
	doQuit = true;
	return 0;
}

/* Handler stubs */
static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->ping(msg,argv[0]->i); return 0; }

// pf 
//static int pfupdate_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {   /* ((OSCHandler *)user_data)->circle(Point(argv[3]->f,argv[4]->f),.30);  */ return 0; }
//static int pfbody_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {   ((OSCHandler *)user_data)->pfbody(Point(argv[2]->f,argv[3]->f)); return 0; }
//static int pfleg_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->pfleg(Point(argv[4]->f,argv[5]->f)); return 0; }
static int pfframe_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->pfframe(argv[0]->i); return 0; }
static int pfsetminx_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setMinX(argv[0]->f);  return 0; }
static int pfsetminy_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setMinY(argv[0]->f); return 0; }
static int pfsetmaxx_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setMaxX(argv[0]->f); return 0; }
static int pfsetmaxy_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setMaxY(argv[0]->f); return 0; }
static int pfaligncorner_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->pfaligncorner(argv[0]->i,argv[1]->i,argv[2]->f,argv[3]->f); return 0; }

OSCHandler::OSCHandler(int port) {
    dbg("OSCHandler",1) << "OSCHandler::OSCHandler()" << std::endl;
    minx=-5; maxx=5;
    miny=0; maxy=0;
    
    gettimeofday(&lastFrameTime,0);
	serverPort=port;

	/* start a new server on OSC port  */
	char cbuf[10];
	sprintf(cbuf,"%d", serverPort);
	s = lo_server_new(cbuf, error);
	if (s==0) {
		fprintf(stderr,"Unable to start server on port %d -- perhaps another instance is running\n", serverPort);
		exit(1);
	}
	printf("Started server on port %d\n", serverPort);

	/* add method that will match the path /quit with no args */
	lo_server_add_method(s, "/quit", "", quit_handler, NULL);

	/* Link management */
	lo_server_add_method(s,"/ping","i",ping_handler,this);




	/* PF */
	lo_server_add_method(s,"/pf/frame","i",pfframe_handler,this);

	lo_server_add_method(s,"/pf/set/minx","f",pfsetminx_handler,this);
	lo_server_add_method(s,"/pf/set/maxx","f",pfsetmaxx_handler,this);
	lo_server_add_method(s,"/pf/set/miny","f",pfsetminy_handler,this);
	lo_server_add_method(s,"/pf/set/maxy","f",pfsetmaxy_handler,this);
	lo_server_add_method(s,"/pf/aligncorner","iiff",pfaligncorner_handler,this);

	/* add method that will match any path and args if they haven't been caught above */
	lo_server_add_method(s, NULL, NULL, generic_handler, this);

	/* Start incoming message processing */
	dbg("OSCHandler",1) << "Creating thread for incoming messages (server=" << s << ")..." << std::flush;
	int rc=pthread_create(&incomingThread, NULL, processIncoming, (void *)this);
	if (rc) {
	    fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	    exit(1);
	}
	dbgn("OSCHandler",1) << "done." << std::endl;
}

OSCHandler::~OSCHandler() {
    int rc=pthread_cancel(incomingThread);
    if (rc)
	dbg("OSCHandler",0) << "pthread_cancel failed with error code " << rc << std::endl;
    lo_server_free(s);
}


// Processing incoming OSC messages in a separate thread
void *OSCHandler::processIncoming(void *arg) {
    SetDebug("pthread:OSCHandler");
    OSCHandler *handler = (OSCHandler *)arg;
    handler->processIncoming();
    return NULL;
}


void OSCHandler::processIncoming() {
    dbg("OSCHandler.processIncoming",1) << "Started: s=" << std::setbase(16) << s << std::setbase(10) << std::endl;
    while (true) {
	int nbytes=lo_server_recv_noblock(s,(int)(1*1000+1));
	dbg("OSCHandler.processIncoming",5) << "Received " << nbytes << " of OSC data" << std::endl;
	if (doQuit)
	    break;
    }
}


void OSCHandler::ping(lo_message msg, int seqnum) {
    std::string url=loutil_address_get_url(lo_message_get_source(msg));
    printf("Got ping from %s\n",url.c_str());
    lo_address addr = lo_address_new_from_url(url.c_str());
    lo_send(addr,"/ack","i",seqnum);
    lo_address_free(addr);
}


void OSCHandler::pfframe(int frame, bool fake) {
    dbg("OSCHandler.pfframe",3) << "pfframe(" << frame << "), lastUpdateFrame=" << lastUpdateFrame << std::endl;
    gettimeofday(&lastFrameTime,0);
    lastUpdateFrame=frame;
}

void OSCHandler::pfaligncorner(int corner, int ncorners, float x, float y) {
    std::vector<Point> align = Calibration::instance()->getAlignment();
    dbg("OSCHandler.pfaligncorner",((ncorners==align.size())?3:1)) << "pfaligncorner(" << corner << "," << ncorners << ", " << x << ", " << y << ")" << std::endl;
    align.resize(ncorners);
    if (corner>=0)
	align[corner]=Point(x,y);
    Calibration::instance()->setAlignment(align);
}

// Called when any of the bounds have been possibly changed
void OSCHandler::updateBounds() {
}
