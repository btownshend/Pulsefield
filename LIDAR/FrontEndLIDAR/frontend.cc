#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include "frontend.h"
#include "urlconfig.h"
#include "sickio.h"

int debug=1;

static void error(int num, const char *msg, const char *path)
{
	fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message , void *) {
	int i;
	printf("Unhandled Message Rcvd: %s (", path);
	for (i=0; i<argc; i++) {
		printf("%c",types[i]);
	}
	printf("): ");
	for (i=0; i<argc; i++) {
		lo_arg_pp((lo_type)types[i], argv[i]);
		printf(", ");
	}
	printf("\n");
	fflush(stdout);
	return 1;
}

static bool doQuit = false;

static int quit_handler(const char *, const char *, lo_arg **, int, lo_message , void *) {
	printf("Received /quit command, quitting\n");
	doQuit = true;
	return 0;
}

/* Handler stubs */
static int start_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(true); return 0; }
static int stop_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(false); return 0; }

static int setFPS_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setFPS(argv[0]->i); return 0; }
static int setRes_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setRes(argv[0]->i,&argv[1]->s); return 0; }
static int setEchoes_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setEchoes(argv[0]->i); return 0; }

static int getRange_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::RANGE,argv[0]->i); return 0; }
static int getReflect_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::REFLECT,argv[0]->i); return 0; }

static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
static int addDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(msg,argv[0]->i); return 0; }
static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(msg,argv[0]->i); return 0; }
static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmAllDest(); return 0; }
static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->ping(msg,argv[0]->i); return 0; }

FrontEnd::FrontEnd(int _nsick) {
	if (debug)
		printf("FrontEnd::FrontEnd()\n");

	frame = 0;
	nsick=_nsick;
	sick = new SickIO*[nsick];
	nechoes=1;

	URLConfig urls("/Users/bst/DropBox/Pulsefield/config/urlconfig.txt");

	serverPort=urls.getPort("FE");
	if (serverPort<0) {
		fprintf(stderr,"Invalid server port retrieved from %s when looking for FE: %d\n", urls.getFilename(),serverPort);
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


	/* Start cameras */
	printf("Initializing with %d sensors...",nsick);fflush(stdout);
	for (int i=0;i<nsick;i++) {
		char ident[20];
		sprintf(ident,"SK%d",i+1);
		int port=urls.getPort(ident);
		if (port<0) {
			fprintf(stderr,"Unable to locate %s in config file %s\n", ident, urls.getFilename());
			exit(1);
		}
		const char *host=urls.getHost(ident);
		sick[i]=new SickIO(i+1,host,port);
		sick[i]->start();
	}
	printf("done\n");fflush(stdout);

	/* add method that will match the path /quit with no args */
	lo_server_add_method(s, "/quit", "", quit_handler, NULL);

	/* add methods */
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

	/* add method that will match any path and args if they haven't been caught above */
	lo_server_add_method(s, NULL, NULL, generic_handler, NULL);

	/* add default destinations */
	// addDest("localhost",7771);

	/* Set to always send only VIS information */
	sendAlways=RANGE;
}

FrontEnd::~FrontEnd() {
	for (int i=0;i<nsick;i++) {
		delete sick[i];
	}
	delete [] sick;
	lo_server_free(s);
}

void FrontEnd::run() {
	int retval;

	int lo_fd = lo_server_get_socket_fd(s);
	if (lo_fd < 0) {
		fprintf(stderr,"lo_server_get_socket_fd failed\n");
		perror("");
		exit(1);
	}
	struct timeval ts1,ts2,lastprocess;
	gettimeofday(&lastprocess,0);   // Keep track of time of last successful frame
	while (true) {  /* Forever */
		fd_set rfds;

		/* Setup file descriptor to watch on */
		FD_ZERO(&rfds);

		/* get the file descriptor of the server socket and watch for messages */
		FD_SET(lo_fd, &rfds);
		int maxfd=lo_fd;

		gettimeofday(&ts1,0);
		if (debug>2) {
			printf("At select after %.3f msec.\n", (ts1.tv_usec-ts2.tv_usec)/1000.0+(ts1.tv_sec-ts2.tv_sec)*1000);
			fflush(stdout);
		}
		struct timeval timeout;
		timeout.tv_usec=1000;
		timeout.tv_sec=0;
		retval = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
		gettimeofday(&ts2,0);
		if (debug>2) {
			printf("Select done after %.3f msec. rfds=0x%lx\n", (ts2.tv_usec-ts1.tv_usec)/1000.0+(ts2.tv_sec-ts1.tv_sec)*1000,*(unsigned long*)&rfds);
			fflush(stdout);
		}
		if (retval == -1) {
			perror("select() error: ");
			exit(1);
		} else if (retval == 0) {
			//fprintf(stderr,"Select timeout\n");
		}

		// Process OSC messages
		if (FD_ISSET(lo_fd, &rfds))
			// Process all queued messages
			while (lo_server_recv_noblock(s, 0) != 0)
				if (doQuit)
					return;

		// Read data from sensors
		bool allValid=true;
		for (int i=0;i<nsick;i++) {
			allValid&=sick[i]->isValid();
		}
		if (allValid)
			processFrames();
		gettimeofday(&lastprocess,0);
	}
	// NOT REACHED
}


void FrontEnd::processFrames() {
	if (debug)
		printf("Processing frame %d\n",frame);
	sendMessages();
	for (int i=0;i<nsick;i++) {
		// clear valid flag so another frame can be read
		sick[i]->clearValid();
	}
	frame=frame+1;
}

// Send all messages that are in the sendOnce list to the destinations
void FrontEnd::sendMessages() {
	if (debug>1)
		printf("sendMessages()  sendOnce=%ld, ndest=%d\n",sendOnce,dests.count());

	sendOnce |= sendAlways;
	for (int i=0;i<dests.count();i++) {
		char cbuf[10];
		printf("Sending messages to %s:%d\n",dests.getHost(i),dests.getPort(i));
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		lo_send(addr,"/vis/beginframe","i",frame);
		for (int c=0;c<nsick;c++) {
			struct timeval ts=sick[c]->getAcquired();
			if (sendOnce & RANGE) {
				// Send range info
				for (int e=0;e<nechoes;e++) {
					const unsigned int *ranges=sick[c]->getRange(e);
					lo_blob data = lo_blob_new(sick[c]->getNumMeasurements()*sizeof(*ranges),ranges);
					if (debug>1)
						printf("Sending RANGE(%d,%d) (N=%d) to %s:%d\n", c, e, sick[c]->getNumMeasurements(), dests.getHost(i),dests.getPort(i));
					lo_send(addr, "/vis/range","iiiiiib",c,sick[c]->getFrame(),ts.tv_sec, ts.tv_usec, e, sick[c]->getNumMeasurements(), data);
					lo_blob_free(data);
				}
			}
			if (sendOnce & REFLECT) {
				for (int e=0;e<nechoes;e++) {
					const unsigned int *reflect=sick[c]->getReflect(e);
					lo_blob data = lo_blob_new(sick[c]->getNumMeasurements()*sizeof(*reflect),reflect);
					if (debug>1)
						printf("Sending REFLECT(%d,%d) (N=%d) to %s:%d\n", c, e, sick[c]->getNumMeasurements(), dests.getHost(i),dests.getPort(i));
					lo_send(addr, "/vis/reflect","iiiiiib",c,sick[c]->getFrame(),ts.tv_sec, ts.tv_usec, e, sick[c]->getNumMeasurements(), data);
					lo_blob_free(data);
				}
			}
		}
		lo_send(addr,"/vis/endframe","i",frame);
		lo_address_free(addr);
	}
	sendOnce=0;
}


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
	for (int i=0;i<dests.count();i++) {
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


void FrontEnd::setFPS(int fps) {
	printf("Setting all sensors to %d FPS\n", fps);
	for (int i=0;i<nsick;i++)
		sick[i]->setScanFreq(fps);
}


void FrontEnd::setRes(int sickid, const char *res) {
	printf("setRes(%d,%s)\n", sickid, res);
	if (sickid<0 || sickid>=nsick) {
		fprintf(stderr,"setRes: bad sensor number: %d\n", sickid);
		return;
	}
	double dres=atof(res);
	sick[sickid]->setScanRes(dres);
}

void FrontEnd::setEchoes(int echoes) {
	if (echoes<1 || echoes>SickIO::MAXECHOES) {
		fprintf(stderr,"setEchoes: bad number of echoes: %d\n",echoes);
	}
	printf("Setting all sensors to return %d echoes\n", echoes);
	nechoes=echoes;
}



void FrontEnd::getStat(long int stat, int mode) {
	printf("getStat(%ld,%d)\n",stat,mode);fflush(stdout);
	if (mode==2)
		sendAlways |= stat;
	else
		sendAlways &= ~stat;

	if (mode>0)
		sendOnce |= stat;
}

void FrontEnd::addDest(const char *host, int port) {
	dests.add(host,port);
}

void FrontEnd::addDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	addDest(host,port);
}

void FrontEnd::rmDest(const char *host, int port) {
	dests.remove(host,port);
}

void FrontEnd::rmDest(lo_message msg, int port) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	rmDest(host,port);
}

void FrontEnd::rmAllDest() {
	dests.removeAll();
}

void FrontEnd::ping(lo_message msg, int seqnum) {
	char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
	printf("Got ping from %s\n",host);
	for (int i=0;i<dests.count();i++) {
		char cbuf[10];
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		lo_send(addr,"/ack","i",seqnum);
		lo_address_free(addr);
	}
}
