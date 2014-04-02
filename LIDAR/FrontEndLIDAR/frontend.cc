#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "frontend.h"
#include "urlconfig.h"
#include "sickio.h"
#include "world.h"
#include "snapshot.h"
#include "vis.h"
#include "dbg.h"

// MATLAB I/O
#include "mat.h"

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
    dbg("FronEnd",1) << "FrontEnd::FrontEnd(" << _nsick << ")" << std::endl;

	matframes=0;
	frame = 0;
	nsick=_nsick;
	if (nsick==0) {
	    sick=new SickIO*[1];
	    sick[0]=0;
	} else
	    sick = new SickIO*[nsick];
	
	world = new World();
	snap = new Snapshot();
	vis = new Vis();
	nechoes=1;
	recording=false;
	recordFD=NULL;

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

	/* Start sending data to Matlab */
	const int clientPort=urls.getPort("MPV");
	const char *clientHost=urls.getHost("MPV");
	dests.add(clientHost, clientPort);

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

	/* Start incoming message processing */
	dbg("FrontEnd",1) << "Creating thread for incoming messages (server=" << s << ")..." << std::flush;
	int rc=pthread_create(&incomingThread, NULL, processIncoming, (void *)s);
	if (rc) {
	    fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	    exit(1);
	}
	dbgn("FrontEnd",1) << "done." << std::endl;
	printf("done\n");

	/* add default destinations */
	// addDest("localhost",7771);

	/* Set to always send only VIS information */
	sendAlways=RANGE;
}

FrontEnd::~FrontEnd() {
    int rc=pthread_cancel(incomingThread);
    if (rc) {
	fprintf(stderr,"pthread_cancel failed with error code %d\n", rc);
    }
    for (int i=0;i<nsick;i++) {
	delete sick[i];
    }
    delete [] sick;
    lo_server_free(s);
}

void FrontEnd::run() {
	int retval;

	struct timeval ts1,ts2,lastprocess;
	gettimeofday(&lastprocess,0);   // Keep track of time of last successful frame
	while (true) {  /* Forever */
		fd_set rfds;

		/* Setup file descriptor to watch on */
		FD_ZERO(&rfds);

		/* get the file descriptor of the server socket and watch for messages */
		//FD_SET(lo_fd, &rfds);
		int maxfd=1;

		gettimeofday(&ts1,0);
		dbg("FrontEnd.run",5) << "At select after " <<  std::setprecision(3) << (ts1.tv_usec-ts2.tv_usec)/1000.0+(ts1.tv_sec-ts2.tv_sec)*1000 << " msec." << std::endl;
		struct timeval timeout;
		timeout.tv_usec=1000;
		timeout.tv_sec=0;
		retval = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
		gettimeofday(&ts2,0);
		dbg("FrontEnd.run",5) << "Select done after " <<  std::setprecision(3) << (ts2.tv_usec-ts1.tv_usec)/1000.0+(ts2.tv_sec-ts1.tv_sec)*1000 << " msec., rfds=0x" << std::setbase(16) << *(unsigned long *)&rfds << std::setbase(10) << std::endl;
		if (retval == -1) {
			perror("select() error: ");
			exit(1);
		} else if (retval == 0) {
			//fprintf(stderr,"Select timeout\n");
		}


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

// Processing incoming OSC messages in a separate thread
void *FrontEnd::processIncoming(void *arg) {
    lo_server s = (lo_server)arg;
    dbg("FrontEnd.processIncoming",1) << "Started: s=" << std::setbase(16) << s << std::setbase(10) << std::endl;
    // Process all queued messages
    while (lo_server_recv(s) != 0)
	if (doQuit)
	    break;
    return NULL;
}

void FrontEnd::processFrames() {
    dbg("FrontEnd.processFrame",1) << "Processing frame " << frame << std::endl;

	sendOnce |= sendAlways;
	for (int c=0;c<nsick;c++) {
	    const unsigned int *range[SickIO::MAXECHOES];
	    const unsigned int *reflect[SickIO::MAXECHOES];
	    for (unsigned int i=0;i<sick[c]->getNumEchoes();i++) {
		range[i]=sick[c]->getRange(i);
		reflect[i]=sick[c]->getReflect(i);
	    }
	    sendVisMessages(sick[c]->getId(),sick[c]->getFrame(),sick[c]->getAcquired(), sick[c]->getNumMeasurements(), sick[c]->getNumEchoes(), range, reflect);
	    // clear valid flag so another frame can be read
	    sick[c]->clearValid();
	}
	vis->update(sick[0]);
	world->track(vis->getClassifier()->getTargets(),*vis,frame,sick[0]->getScanFreq());
	world->sendMessages(dests,sick[0]->getAcquired());

	sendOnce=0;
	if (recording)
	    recordFrame();

	frame=frame+1;
}

void FrontEnd::sendVisMessages(int id, unsigned int frame, const struct timeval &acquired, int nmeasure, int necho, const unsigned int **ranges, const unsigned int **reflect) {
    dbg("FrontEnd.sendVisMessages",5) << "sendOnce=0x" << std::setbase(16) << sendOnce << std::setbase(10)  << ", ndest=" << dests.count() << std::endl;

	for (int i=0;i<dests.count();i++) {
		char cbuf[10];
		dbg("FrontEnd.sendVisMessages",6) << "Sending messages to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
		sprintf(cbuf,"%d",dests.getPort(i));
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		lo_send(addr,"/vis/beginframe","i",frame);
		if (sendOnce & RANGE) {
		    // Send range info
		    for (int e=0;e<nechoes;e++) {
			lo_blob data = lo_blob_new(nmeasure*sizeof(*ranges[e]),ranges[e]);
			dbg("FrontEnd.sendVisMessages",6) << "Sending RANGE(" << id << "," << e << ") (N=" << nmeasure << ") to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
			lo_send(addr, "/vis/range","iiiiiib",id,frame,acquired.tv_sec, acquired.tv_usec, e, nmeasure, data);
			lo_blob_free(data);
		    }
		}
		if (sendOnce & REFLECT) {
		    for (int e=0;e<nechoes;e++) {
			lo_blob data = lo_blob_new(nmeasure*sizeof(*reflect[e]),reflect[e]);
			dbg("FrontEnd.sendVisMessages",6) << "Sending REFLECT(" << id << "," << e << ") (N=" << nmeasure << ") to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
			lo_send(addr, "/vis/reflect","iiiiiib",id,frame,acquired.tv_sec, acquired.tv_usec, e, nmeasure, data);
			lo_blob_free(data);
		    }
		}
		lo_send(addr,"/vis/endframe","i",frame);
		lo_address_free(addr);
	}
}

void FrontEnd::recordFrame() {
    assert(recording && recordFD>=0);
    
    for (int c=0;c<nsick;c++) {
	struct timeval ts=sick[c]->getAcquired();
	fprintf(recordFD,"%d %d %ld %d %d %d\n",sick[c]->getId(),sick[c]->getFrame(),ts.tv_sec,ts.tv_usec,nechoes,sick[c]->getNumMeasurements());
	for (int e=0;e<nechoes;e++) {
	    const unsigned int *ranges=sick[c]->getRange(e);
	    fprintf(recordFD,"D%d ",e);
	    for (unsigned int i=0;i<sick[c]->getNumMeasurements();i++)
		fprintf(recordFD,"%d ",ranges[i]);
	    fprintf(recordFD,"\n");
	}
	for (int e=0;e<nechoes;e++) {
	    const unsigned int *reflect=sick[c]->getReflect(e);
	    fprintf(recordFD,"R%d ",e);
	    for (unsigned int i=0;i<sick[c]->getNumMeasurements();i++)
		fprintf(recordFD,"%d ",reflect[i]);
	    fprintf(recordFD,"\n");
	}
    }
}

int FrontEnd::startRecording(const char *filename) {
    recordFD = fopen(filename,"w");
    if (recordFD == NULL) {
	fprintf(stderr,"Unable to open recording file %s for writing\n", filename);
	return -1;
    }
    printf("Recording into %s\n", filename);
    recording=true;
    return 0;
}

void FrontEnd::stopRecording() {
    (void)fclose(recordFD);
    recording=false;
}

int FrontEnd::playFile(const char *filename,bool singleStep,float speedFactor) {
    printf("Playing back recording from %s\n", filename);
    FILE *fd=fopen(filename,"r");
    if (fd == NULL) {
	fprintf(stderr,"Unable to open playback file %s for reading\n", filename);
	return -1;
    }
    unsigned int range[SickIO::MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
    unsigned int reflect[SickIO::MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
    const unsigned int *rangeref[SickIO::MAXECHOES];
    const unsigned int *reflectref[SickIO::MAXECHOES];
    for (int i=0;i<SickIO::MAXECHOES;i++) {
	rangeref[i]=range[i];
	reflectref[i]=reflect[i];
    }

    struct timeval lastfile;
    struct timeval lastnow;
    int frameStep=0;
    int lastcframe=-1;

    while (true) {
	sendOnce |= sendAlways;
	int cid,cframe,nechoes,nmeasure;
	struct timeval acquired;
	if (EOF==fscanf(fd,"%d %d %ld %d %d %d\n",&cid,&cframe,&acquired.tv_sec,&acquired.tv_usec,&nechoes,&nmeasure)) {
	    printf("EOF on %s\n",filename);
	    break;
	}
	if (cframe!=lastcframe+1 && lastcframe!=-1)
	    fprintf(stderr,"Input file skips frames %d-%d\n",lastcframe+1,cframe-1);

	lastcframe=cframe;
	while (singleStep && frameStep<=0) {
	    printf("Num frames to step? ");
	    char buf[100];
	    fgets(buf,sizeof(buf)-1,stdin);
	    if (buf[0]==0xa)
		frameStep=1;
	    else
		frameStep=atoi(buf);
	    printf("Advancing %d frames\n", frameStep);
	} 
	frameStep--;

	struct timeval now;
	gettimeofday(&now,0);
	
	long int waittime=(acquired.tv_sec-lastfile.tv_sec-(now.tv_sec-lastnow.tv_sec))*1000000+(acquired.tv_usec-lastfile.tv_usec-(now.tv_usec-lastnow.tv_usec));
	waittime=waittime*speedFactor;
	if (waittime >1000 && waittime<1000000) {
	    usleep(waittime);
	}

	lastnow=now;
	lastfile=acquired;

	assert(nechoes>=1 && nechoes<=SickIO::MAXECHOES);
	assert(nmeasure>0 && nmeasure<=SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS);

	for (int e=0;e<nechoes;e++) {
	    int echo;
	    fscanf(fd,"D%d ",&echo);
	    assert(echo>=0 && echo<nechoes);
	    for (int i=0;i<nmeasure;i++)
		fscanf(fd,"%d ",&range[e][i]);
	}
	for (int e=0;e<nechoes;e++) {
	    int echo;
	    fscanf(fd,"R%d ",&echo);
	    assert(echo>=0 && echo<nechoes);
	    for (int i=0;i<nmeasure;i++)
		fscanf(fd,"%d ",&reflect[e][i]);
	}
	if (cframe%100==0)
	    printf("Playing frame %d\n",cframe);
	sendVisMessages(cid,cframe, acquired,  nmeasure, nechoes, &rangeref[0], &reflectref[0]);
	if (!sick[0])
	    sick[0]=new SickIO();
	
	sick[0]->set(cid,cframe, acquired,  nmeasure, nechoes, range,reflect);

	
	vis->update(sick[0]);
	world->track(vis->getClassifier()->getTargets(),*vis,cframe,sick[0]->getScanFreq());

	if (matframes>0) {
	    snap->append(vis,world);
	    
	    if (cframe==matframes) {
		snap->save(matfile);
		break;
	    }
	}

	sendOnce=0;
    }
    fclose(fd);
    return 0;
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
    dbg("FrontEnd.setFPS",1) << "Setting all sensors to " << fps << " FPS" << std::endl;
	for (int i=0;i<nsick;i++)
		sick[i]->setScanFreq(fps);
}


void FrontEnd::setRes(int sickid, const char *res) {
    dbg("FrontEnd.setRes",1) << "Setting sensors " << sickid << " to resolution " << res  << " degrees." << std::endl;
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
	dbg("FrontEnd.setEchoes",1) << "Setting all sensors to return " << echoes << " echoes." << std::endl;
	nechoes=echoes;
}



void FrontEnd::getStat(long int stat, int mode) {
    dbg("FrontEnd.getStat",1) << "getStat(" << stat << "," << mode << ")" << std::endl;
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
