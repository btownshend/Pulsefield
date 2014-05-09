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
#include "video.h"
#include "point.h"

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
	    fprintf(stderr,"%c",types[i]);
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

// Link management
static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
static int addDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->addDest(msg,argv[0]->i); return 0; }
static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmDest(msg,argv[0]->i); return 0; }
static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->rmAllDest(); return 0; }
static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->ping(msg,argv[0]->i); return 0; }

// Laser settings
static int setPPS_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setPPS(argv[0]->f); return 0; }
static int setBlanking_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setBlanking(argv[0]->i,argv[0]->i); return 0; }
static int setSkew_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setSkew(argv[0]->i); return 0; }

// Attributes
static int setColor_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setColor(Color(argv[0]->f,argv[1]->f,argv[2]->f)); return 0; }
static int setDensity_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setDensity(argv[0]->f); return 0; }
static int setAttribute_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setAttribute(&argv[0]->s,argv[1]->f); return 0; }

// Primitives
static int circle_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->circle(Point(argv[0]->f,argv[1]->f),argv[2]->f); return 0; }
static int arc_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->arc(Point(argv[0]->f,argv[1]->f),Point(argv[2]->f,argv[3]->f),argv[4]->f); return 0; }
static int cubic_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->cubic(Point(argv[0]->f,argv[1]->f),Point(argv[2]->f,argv[3]->f),Point(argv[4]->f,argv[5]->f),Point(argv[6]->f,argv[7]->f)); return 0; }
static int line_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->line(Point(argv[0]->f,argv[1]->f),Point(argv[2]->f,argv[3]->f)); return 0; }

// Transforms
static int map_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->map(Point(argv[0]->f,argv[1]->f),Point(argv[2]->f,argv[3]->f)); return 0; }
static int setTransform_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->setTransform(); return 0; }

// Draw
static int update_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->update(argv[0]->i); return 0; }

// pf 
static int pfupdate_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->circle(Point(argv[3]->f,argv[4]->f),.30); return 0; }
static int pfframe_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->pfframe(); return 0; }
static int pfsetminx_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->minx=argv[0]->f; return 0; }
static int pfsetminy_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->miny=argv[0]->f; return 0; }
static int pfsetmaxx_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->maxx=argv[0]->f; return 0; }
static int pfsetmaxy_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((OSCHandler *)user_data)->maxy=argv[0]->f; return 0; }

OSCHandler::OSCHandler(int _unit, Laser *_laser, Video *_video): currentColor(1.0,1.0,1.0) {
    dbg("OSCHandler",1) << "OSCHandler::OSCHandler(" << _unit << ")" << std::endl;
    unit=_unit;
    laser=_laser;
    video=_video;
    currentDensity=1.0;
    minx=-5; maxx=5;
    miny=0; maxy=0;

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

	/* Link management */
	lo_server_add_method(s,"/laser/dest/add","si",addDest_handler,this);
	lo_server_add_method(s,"/laser/dest/add/port","i",addDestPort_handler,this);
	lo_server_add_method(s,"/laser/dest/remove","si",rmDest_handler,this);
	lo_server_add_method(s,"/laser/dest/remove/port","i",rmDestPort_handler,this);
	lo_server_add_method(s,"/laser/dest/clear","",rmAllDest_handler,this);
	lo_server_add_method(s,"/ping","i",ping_handler,this);

	/* Attributes */
	lo_server_add_method(s,"/laser/set/color","fff",setColor_handler,this);
	lo_server_add_method(s,"/laser/set/density","f",setDensity_handler,this);
	lo_server_add_method(s,"/laser/set/attribute","sf",setAttribute_handler,this);

	/* Laser settings */
	lo_server_add_method(s,"/laser/set/pps","f",setPPS_handler,this);
	lo_server_add_method(s,"/laser/set/blanking","ii",setBlanking_handler,this);
	lo_server_add_method(s,"/laser/set/skew","i",setSkew_handler,this);

	/* Primitives */
	lo_server_add_method(s,"/laser/circle","fff",circle_handler,this);
	lo_server_add_method(s,"/laser/arc","fffff",arc_handler,this);
	lo_server_add_method(s,"/laser/bezier/cubic","ffffffff",cubic_handler,this);
	lo_server_add_method(s,"/laser/line","ffff",line_handler,this);

	/* Transforms */
	lo_server_add_method(s,"/laser/map","ffff",map_handler,this);
	lo_server_add_method(s,"/laser/settransform","",setTransform_handler,this);

	/* Draw */
	lo_server_add_method(s,"/laser/update","i",update_handler,this);


	/* PF */
	lo_server_add_method(s,"/pf/frame","i",pfframe_handler,this);
	lo_server_add_method(s,"/pf/update","ififfffffiii",pfupdate_handler,this);
	lo_server_add_method(s,"/pf/set/minx","f",pfsetminx_handler,this);
	lo_server_add_method(s,"/pf/set/maxx","f",pfsetmaxx_handler,this);
	lo_server_add_method(s,"/pf/set/miny","f",pfsetminy_handler,this);
	lo_server_add_method(s,"/pf/set/maxy","f",pfsetmaxy_handler,this);

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


void OSCHandler::setPPS(int pps) {
    dbg("OSCHandler.setPPS",1) << "Setting PPS to " << pps << " PPS" << std::endl;
    // TODO:
}

void OSCHandler::setBlanking(int before, int after) {
    dbg("OSCHandler.setBlanking",1) << "Setting blanking to " <<before << ", " << after << std::endl;
    // TODO:
}

void OSCHandler::setSkew(int skew) {
    dbg("OSCHandler.setSkew",1) << "Setting skew to " << skew  << std::endl;
    // TODO:
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

void OSCHandler::setColor(Color c ) {
    currentColor=c;
}

void OSCHandler::setDensity(float d ) {
    currentDensity=d;
}

void OSCHandler::setAttribute(const char *attr, float value ) {
    // TODO
}

void OSCHandler::circle(Point center, float radius ) {
    drawing.drawCircle(center,radius,currentColor);
}

void OSCHandler::arc(Point center, Point pt, float angle ) {
    drawing.drawArc(center,pt,angle,currentColor);
}

void OSCHandler::line(Point p1, Point p2) {
    drawing.drawLine(p1,p2,currentColor);
}

void OSCHandler::cubic(Point p1, Point p2, Point p3, Point p4) {
    drawing.drawCubic(p1,p2,p3,p4,currentColor);
}

void OSCHandler::map(Point p1, Point p2) {
    drawing.addToMap(p1,p2);
}

void OSCHandler::setTransform() {
    drawing.setTransform();
}

void OSCHandler::update(int nPoints ) {
    std::vector<etherdream_point> pts=drawing.getPoints(nPoints);
    if (pts.size()>=2) {
	laser->update(pts);
	video->update(pts,drawing.getTransform());
    }
    drawing.clear();
}

void OSCHandler::pfframe() {
    // Setup mapping
    drawing.addToMap(Point(-32767,-32767),Point(minx,maxy));
    drawing.addToMap(Point(32767,-32767),Point(maxx,maxy));
    drawing.addToMap(Point(-32767,32767),Point(minx,miny));
    drawing.addToMap(Point(32767,32767),Point(maxx,miny));
    drawing.setTransform();
    // Update
    update(500);
}
