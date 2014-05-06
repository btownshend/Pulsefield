#include <iomanip>
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
#include "urlconfig.h"
#include "laser.h"
#include "point.h"

int debug=1;

static void error(int num, const char *msg, const char *path)
{
	fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message , void *) {
	int i;
	fprintf(stderr, "Unhandled Message Rcvd: %s (", path);
	for (i=0; i<argc; i++) {
		printf("%c",types[i]);
	}
	fprintf(stderr, "): ");
	for (i=0; i<argc; i++) {
		lo_arg_pp((lo_type)types[i], argv[i]);
		fprintf(stderr, ", ");
	}
	fprintf(stderr,"\n");
	fflush(stderr);
	return 1;
}

static bool doQuit = false;

static int quit_handler(const char *, const char *, lo_arg **, int, lo_message , void *) {
	printf("Received /quit command, quitting\n");
	doQuit = true;
	return 0;
}

/* Handler stubs */
static int start_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->startStop(true); return 0; }
static int stop_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->startStop(false); return 0; }

static int setFPS_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setFPS(argv[0]->i); return 0; }
static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
static int addDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->addDest(msg,argv[0]->i); return 0; }
static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmDest(msg,argv[0]->i); return 0; }
static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmAllDest(); return 0; }
static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->ping(msg,argv[0]->i); return 0; }
static int circle_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->circle(msg,argv[0]->f,argv[1]->f,argv[2]->f,argv[3]->f,argv[4]->f,argv[5]->f); return 0; }
static int line_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->line(msg,argv[0]->f,argv[1]->f,argv[2]->f,argv[3]->f,argv[4]->f,argv[5]->f,argv[6]->f); return 0; }
static int map_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->map(msg,argv[0]->f,argv[1]->f,argv[2]->f,argv[3]->f); return 0; }
static int setTransform_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setTransform(msg); return 0; }
static int update_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->update(msg,argv[0]->i); return 0; }

OSCHandler::OSCHandler(int _unit, Laser *_laser) {
    dbg("OSCHandler",1) << "OSCHandler::OSCHandler(" << _unit << ")" << std::endl;
    unit=_unit;
    laser=_laser;

	URLConfig urls("/Users/bst/DropBox/Pulsefield/config/urlconfig.txt");

	serverPort=urls.getPort("LASER");
	if (serverPort<0) {
		fprintf(stderr,"Invalid server port retrieved from %s when looking for LASER: %d\n", urls.getFilename(),serverPort);
		exit(1);
	}


	/* start a new server on OSC port  */
	char cbuf[10];
	sprintf(cbuf,"%d", serverPort);
	s = lo_server_new(cbuf, error);
	if (s==0) {
		fprintf(stderr,"Unable to start server on port %d -- perhaps another instance is running\n", serverPort);
		exit(1);
	}
	printf("Started server on port %d\n", serverPort);

	/* Start sending data to hardwired OSC destinations */
	const char *targets[]={"VD"};
	for (unsigned int i=0;i<sizeof(targets)/sizeof(targets[0]);i++) {
	    int clientPort=urls.getPort(targets[i]);
	    const char *clientHost=urls.getHost(targets[i]);
	    if (clientHost==0 || clientPort==-1)
		fprintf(stderr,"Unable to location %s in urlconfig.txt\n", targets[i]);
	    else
		addDest(clientHost, clientPort);
	}


	/* add method that will match the path /quit with no args */
	lo_server_add_method(s, "/quit", "", quit_handler, NULL);

	/* add methods */
	lo_server_add_method(s,"/laser/start","",start_handler,this);
	lo_server_add_method(s,"/laser/stop","",stop_handler,this);

	lo_server_add_method(s,"/laser/set/fps","i",setFPS_handler,this);
	lo_server_add_method(s,"/laser/circle","ffffff",circle_handler,this);
	lo_server_add_method(s,"/laser/line","fffffff",line_handler,this);
	lo_server_add_method(s,"/laser/map","ffff",map_handler,this);
	lo_server_add_method(s,"/laser/settransform","",setTransform_handler,this);
	lo_server_add_method(s,"/laser/update","i",update_handler,this);
	lo_server_add_method(s,"/ping","i",ping_handler,this);


	lo_server_add_method(s,"/laser/dest/add","si",addDest_handler,this);
	lo_server_add_method(s,"/laser/dest/add/port","i",addDestPort_handler,this);
	lo_server_add_method(s,"/laser/dest/remove","si",rmDest_handler,this);
	lo_server_add_method(s,"/laser/dest/remove/port","i",rmDestPort_handler,this);
	lo_server_add_method(s,"/laser/dest/clear","",rmAllDest_handler,this);

	/* add method that will match any path and args if they haven't been caught above */
	lo_server_add_method(s, NULL, NULL, generic_handler, NULL);

	/* Start incoming message processing */
	dbg("OSCHandler",1) << "Creating thread for incoming messages (server=" << s << ")..." << std::flush;
	int rc=pthread_create(&incomingThread, NULL, processIncoming, (void *)s);
	if (rc) {
	    fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	    exit(1);
	}
	dbgn("OSCHandler",1) << "done." << std::endl;
}

OSCHandler::~OSCHandler() {
    int rc=pthread_cancel(incomingThread);
    if (rc)
	dbg("OSCHandler",1) << "pthread_cancel failed with error code " << rc << std::endl;
    lo_server_free(s);
}


// Processing incoming OSC messages in a separate thread
void *OSCHandler::processIncoming(void *arg) {
    lo_server s = (lo_server)arg;
    dbg("OSCHandler.processIncoming",1) << "Started: s=" << std::setbase(16) << s << std::setbase(10) << std::endl;
    // Process all queued messages
    while (lo_server_recv(s) != 0)
	if (doQuit)
	    break;
    return NULL;
}

void OSCHandler::startStop(bool start) {
	printf("OSCHandler: %s\n", start?"start":"stop");

	// Send status update message
	for (int i=0;i<dests.size();i++) {
		char cbuf[10];
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		if (start)
			lo_send(addr,"/laser/started","");
		else
			lo_send(addr,"/laser/stopped","");
		lo_address_free(addr);
	}
}


void OSCHandler::setFPS(int fps) {
    dbg("OSCHandler.setFPS",1) << "Setting FPS to " << fps << " FPS" << std::endl;
}

int OSCHandler::getFPS() const {
    //fprintf(stderr,"Warning: assuming recorded file is at 50 FPS\n");
    return 50;
}


void OSCHandler::addDest(const char *host, int port) {
	dests.add(host,port);
}

void OSCHandler::addDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	addDest(host,port);
}

void OSCHandler::rmDest(const char *host, int port) {
	dests.remove(host,port);
}

void OSCHandler::rmDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	rmDest(host,port);
}

void OSCHandler::rmAllDest() {
	dests.removeAll();
}

void OSCHandler::ping(lo_message msg, int seqnum) {
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

void OSCHandler::circle(lo_message msg, float x, float y, float radius, float r, float g, float b ) {
    drawing.drawCircle(Point(x,y),radius,Color(r,g,b));
}

void OSCHandler::line(lo_message msg, float x1, float y1, float x2, float y2, float r, float g, float b) {
    drawing.drawLine(Point(x1,y1),Point(x2,y2),Color(r,g,b));
}

void OSCHandler::map(lo_message msg, float x1, float y1, float x2, float y2) {
    drawing.addToMap(Point(x1,y1),Point(x2,y2));
}

void OSCHandler::setTransform(lo_message msg) {
    drawing.setTransform();
}

void OSCHandler::update(lo_message msg, int nPoints ) {
    std::vector<etherdream_point> pts=drawing.getPoints(nPoints);
    laser->update(pts);
    drawing.clear();
}
