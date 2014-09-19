#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <numeric>

#include "frontend.h"
#include "urlconfig.h"
#include "sickio.h"
#include "world.h"
#include "snapshot.h"
#include "vis.h"
#include "dbg.h"
#include "parameters.h"

// MATLAB I/O
#include "mat.h"

int debug=1;

static void error(int num, const char *msg, const char *path)
{
	fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}


FrontEnd::FrontEnd(int _nsick,float maxRange,int argc, const char *argv[]) {
    dbg("FrontEnd",1) << "FrontEnd::FrontEnd(" << _nsick << ")" << std::endl;

    for (int i=0;i<argc;i++)
	arglist.push_back(argv[i]);

    starttime.tv_sec=0;
    starttime.tv_usec=0;

	matframes=0;
	frame = 0;
	nsick=_nsick;
	if (nsick==0) {
	    sick=new SickIO*[1];
	    sick[0]=0;
	} else
	    sick = new SickIO*[nsick];
	
	world = new World(maxRange);
	vis = new Vis();
	snap=NULL;  // If needed, set in matsave()
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

	/* Start sending data to hardwired OSC destinations */
	//	const char *targets[]={"VD","LASER"};
	const char *targets[]={ "VIS","COND","LAN","REC","COND2","VIS2","LASER"};
	for (unsigned int i=0;i<sizeof(targets)/sizeof(targets[0]);i++) {
	    int clientPort=urls.getPort(targets[i]);
	    const char *clientHost=urls.getHost(targets[i]);
	    if (clientHost==0 || clientPort==-1)
		fprintf(stderr,"Unable to locate %s in urlconfig.txt\n", targets[i]);
	    else
		addDest(clientHost, clientPort);
	}
	
	/* Setup touchOSC sending */
	int touchOSCPort=urls.getPort("TO");
	const char *touchOSCHost=urls.getHost("TO");
	if (touchOSCPort==-1 || touchOSCHost==0) {
	    fprintf(stderr,"Unable to locate TO in urlconfig.txt\n");
	    touchOSC=0;
	} else {
	    dbg("FrontEnd",1) << "Sending to touchOSC at " << touchOSCHost << ":" << touchOSCPort << std::endl;
	    char cbuf[10];
	    sprintf(cbuf,"%d",touchOSCPort);
	    touchOSC = lo_address_new(touchOSCHost, cbuf);
	    sendUIMessages();
	}

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

	addHandlers();

	/* Start incoming message processing */
	dbg("FrontEnd",1) << "Creating thread for incoming messages (server=" << s << ")..." << std::flush;
	int rc=pthread_create(&incomingThread, NULL, processIncoming, (void *)s);
	if (rc) {
	    fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	    exit(1);
	}
	dbgn("FrontEnd",1) << "done." << std::endl;

	/* add default destinations */
	// addDest("localhost",7771);

	/* Set to always send only PF information */
	sendAlways=PF|RANGE;
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

void FrontEnd::matsave(const std::string &filename, int frames) {
    matfile=filename; 
    matframes=frames;
    snap = new Snapshot(arglist);
}

void FrontEnd::run() {
    while (true) {  /* Forever */
	// Read data from sensors
	sick[0]->lock();	// Needs to be modified to support multiple LIDARs
	sick[0]->waitForFrame();
	bool allValid=true;
	for (int i=0;i<nsick;i++) {
	    allValid&=sick[i]->isValid();
	}
	if (allValid)
	    processFrames();
	sick[0]->unlock();
    }
    // NOT REACHED
}

// Processing incoming OSC messages in a separate thread
void *FrontEnd::processIncoming(void *arg) {
    lo_server s = (lo_server)arg;
    SetDebug("pthread:OSCIncoming");
    dbg("FrontEnd.processIncoming",1) << "Started: s=" << std::setbase(16) << s << std::setbase(10) << std::endl;
    // Process all queued messages
    while (lo_server_recv(s) != 0)
	if (doQuit)
	    break;
    return NULL;
}

void FrontEnd::processFrames() {
    dbg("FrontEnd.processFrame",1) << "Processing frame " << frame << std::endl;
	
	char dbgstr[100];
	sprintf(dbgstr,"Frame.%d",frame);
	bool tmpDebug=false;
	if (DebugCheck(dbgstr,20)) {
	    PushDebugSettings();
	    SetDebug("20");
	    tmpDebug=true;
	    dbg("FrontEnd",1) << "Temporarily set debug level to 20 for frame " << frame << std::endl;
	}

	sendOnce |= sendAlways;
	for (int c=0;c<nsick;c++) {
	    const unsigned int *range[SickIO::MAXECHOES];
	    const unsigned int *reflect[SickIO::MAXECHOES];
	    for (unsigned int i=0;i<sick[c]->getNumEchoes();i++) {
		range[i]=sick[c]->getRange(i);
		reflect[i]=sick[c]->getReflect(i);
	    }
	    sendVisMessages(sick[c]->getId(),sick[c]->getFrame(),sick[c]->getAcquired(), sick[c]->getNumMeasurements(), sick[c]->getNumEchoes(), range, reflect);
	    if (recording)
		recordFrame();
	    // clear valid flag so another frame can be read
	    sick[c]->clearValid();
	}
	currenttime=sick[0]->getAcquired();
	if (starttime.tv_sec==0)
	    starttime=currenttime;
	vis->update(sick[0]);
	double elapsed=(currenttime.tv_sec-starttime.tv_sec)+(currenttime.tv_usec-starttime.tv_usec)*1e-6;
	world->track(*vis,frame,sick[0]->getScanFreq(),elapsed);
	sendMessages(elapsed);
	sendOnce=0;
	dbg("FrontEnd",2) << "Bounds=[" << world->getMinX() << "," << world->getMaxX() << "," << world->getMinY() << "," << world->getMaxY() << "]" << std::endl;
	if (tmpDebug) {
	    dbg("FrontEnd",1) << "End of frame " << frame << ", restoring debug levels" << std::endl;
	    PopDebugSettings();
	}
	frame=frame+1;
}

void FrontEnd::sendVisMessages(int id, unsigned int frame, const struct timeval &acquired, int nmeasure, int necho, const unsigned int **ranges, const unsigned int **reflect) {
    if (! (sendOnce & (RANGE|REFLECT)))
	return;
    dbg("FrontEnd.sendVisMessages",5) << "sendOnce=0x" << std::setbase(16) << sendOnce << std::setbase(10)  << ", ndest=" << dests.size() << std::endl;

	for (int i=0;i<dests.size();i++) {
		char cbuf[10];
		dbg("FrontEnd.sendVisMessages",6) << "Sending messages to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
		sprintf(cbuf,"%d",dests.getPort(i));
		if (dests.getPort(i)==7010)
		  continue;
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

int FrontEnd::playFile(const char *filename,bool singleStep,float speedFactor,bool overlayLive,int frame1, int frameN, bool savePerfData) {
    printf("Playing back recording from %s\n", filename);
    FILE *fd=fopen(filename,"r");
    if (fd == NULL) {
	fprintf(stderr,"Unable to open playback file %s for reading\n", filename);
	return -1;
    }
    unsigned int range[SickIO::MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
    unsigned int reflect[SickIO::MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];

    struct timeval startfile;
    struct timeval starttime;
    gettimeofday(&starttime,0);
    int frameStep=0;
    int lastframe=-1;
    float maxProcTime=0;
    float totalProcTime=0;
    float totalallProcTime=0;
    int nProcTime=0;
    int nallProcTime=0;
    std::vector<float> allperf;
    int minPerfFrame=1000000;
    int maxPerfFrame=-1;
    int maxPeople=0;
    int activeFrames=0;

    while (true) {
	int cid,nechoes,nmeasure;
	struct timeval acquired;
	int nread;
	if (EOF==(nread=fscanf(fd,"%d %d %ld %d %d %d\n",&cid,&frame,&acquired.tv_sec,&acquired.tv_usec,&nechoes,&nmeasure))) {
	    printf("EOF on %s\n",filename);
	    break;
	}
	if (nread!=6)  {
	    std::cerr << "Error scaning input file, read " << nread << " entries when 6 were expected" << std::endl;
	    return -1;
	}
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
	    
	if (lastframe==-1) 
	    // Initialize file start time for reference
	    startfile=acquired;

	if (frame1==-1 && frameN!=-1) {
	    // Just first frameN frames
	    frame1=frame;
	    frameN+=frame-1;
	}
	if (frame1!=-1 && frame<frame1)
	    continue;
	if (frameN!=-1 && frame>frameN)
	    break;

	if (frame!=lastframe+1 && lastframe!=-1) {
	    if (lastframe+1 == frame-1)
		fprintf(stderr,"Input file skips frame %d\n",lastframe+1);
	    else
		fprintf(stderr,"Input file skips frames %d-%d\n",lastframe+1,frame-1);
	}

	lastframe=frame;
	if (!overlayLive) {
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
	
	    long int waittime=(acquired.tv_sec-startfile.tv_sec-speedFactor*(now.tv_sec-starttime.tv_sec))*1000000+(acquired.tv_usec-startfile.tv_usec-speedFactor*(now.tv_usec-starttime.tv_usec));
	    if (waittime >1000) {
		// When doing an overlay, timing is driven by file and data is sampled whenever overlay data is ready, if nothing is valid a frame is skipped
		usleep(waittime);
	    }
	} /* else use real-time timing */

	
	struct timeval processStart; gettimeofday(&processStart,0);
	if (overlayLive) {
	    sick[0]->lock();
	    sick[0]->waitForFrame();
	    assert(sick[0]->isValid());
	    sick[0]->overlay(cid,frame, acquired,  nmeasure, nechoes, range,reflect);
	    processFrames();
	    assert(!sick[0]->isValid());
	    sick[0]->unlock();
	} else {
	    if (nsick==0) {
		sick[0]=new SickIO();
		nsick=1;
	    }
	    sick[0]->lock();
	    sick[0]->set(cid,frame, acquired,  nmeasure, nechoes, range,reflect);
	    assert(sick[0]->isValid());
	    processFrames();
	    assert(!sick[0]->isValid());
	    sick[0]->unlock();
	}
	struct timeval processDone; gettimeofday(&processDone,0);
	float procTime=(processDone.tv_sec-processStart.tv_sec)+(processDone.tv_usec-processStart.tv_usec)/1e6;
	totalProcTime+=procTime;
	totalallProcTime+=procTime;
	maxProcTime=std::max(maxProcTime,procTime);
	nProcTime++;
	nallProcTime++;

	if (frame%200==0) {
	    dbg("frontend",1) << "Frame " << frame << ": mean frame proessing time= " <<  totalProcTime/nProcTime << ", max=" << maxProcTime << ", max FPS=" << nProcTime/totalProcTime << std::endl;
	    printf("Playing frame %d with mean processing time=%.1f ms (%.0f FPS), max=%.1f ms\n",frame,totalProcTime/nProcTime*1000,nProcTime/totalProcTime,maxProcTime*1000);
	    nProcTime=0;
	    maxProcTime=0;
	    totalProcTime=0;
	}

	if (!matfile.empty()) {
	    snap->append(vis,world);
	    
	    if (matframes>0 && frame>=matframes)
		// Do final output below
		break;

	    if (frame%2000 == 0) {
		// Break up the output
		char tmpfile[1000];
		sprintf(tmpfile,"%s-%d.mat",matfile.c_str(),frame);
		snap->save(tmpfile);
		snap->clear();
	    }
	}
	minPerfFrame=std::min(minPerfFrame,frame-1);
	maxPerfFrame=std::max(maxPerfFrame,frame-1);
	if (world->numPeople() > 0) {
	    maxPeople=std::max(world->numPeople(),maxPeople);
	    std::vector<float> framePerf=world->getFramePerformance();
	    allperf.insert(allperf.end(),framePerf.begin(),framePerf.end());
	    activeFrames++;
	}
    }
    fclose(fd);

    if (savePerfData) {
	// Write performance data
	static const char *perfFilename="performance.csv";
	FILE *perfFD=fopen(perfFilename,"a");
	if (perfFD == NULL) {
	    fprintf(stderr,"Unable to open performance file %s for appending\n", perfFilename);
	} else {
	    std::cout << "Writing performance data to " << perfFilename << std::endl;
	    // Get current time
	    time_t now;
	    time(&now);
	    struct tm *timeptr=localtime(&now);
	    char datestr[100];
	    strftime(datestr,sizeof(datestr),"%d-%h-%Y %H:%M:%S",timeptr);

	    // Get current GIT revision
	    const char *command = "git log -1 --format='%h' 2>&1";
	    FILE *fp = popen(command, "r");
	    char gitrev[50];
	    if (fp == NULL) {
		std::cerr << "Failed to run " << command << std::endl;
		strcpy(gitrev,"?");
	    } else {
		(void)fgets(gitrev, sizeof(gitrev)-1, fp);
		if (strlen(gitrev)>0)
		    gitrev[strlen(gitrev)-1]=0;  // Remove newline
	    }
	    pclose(fp);

	    float meanfps=nallProcTime/totalallProcTime;
	    std::sort(allperf.begin(),allperf.end());
	    float mean=std::accumulate(allperf.begin(),allperf.end(),0)/allperf.size();
	    std::string allargs=arglist[0];
	    for (unsigned int i=1;i<arglist.size();i++)
		allargs+=" "+arglist[i];
	    fprintf(perfFD,"\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,%d,%d,%ld,%g",allargs.c_str(),filename, gitrev,datestr, minPerfFrame, maxPerfFrame, activeFrames,maxPeople, world->getLastID(),allperf.size(), sqrt(mean)/UNITSPERM);
	    fprintf(perfFD,",%.2f",meanfps);
	    float prctiles[]={0,0.5,0.9,0.95,0.99,1.0};
	    for (int i=0;i<sizeof(prctiles)/sizeof(prctiles[0]);i++) {
		int pos=(int)((allperf.size()-1)*prctiles[i]+0.5);
		fprintf(perfFD,",%.0f,%f",100*prctiles[i],sqrt(allperf[pos])/UNITSPERM);
	    }
    
	    fprintf(perfFD,"\n");
	    fclose(perfFD);
	}
    }

    if (!matfile.empty()) {
	char tmpfile[1000];
	sprintf(tmpfile,"%s-%d.mat",matfile.c_str(),frame);
	snap->save(tmpfile);
	snap->clear();
    }
    return 0;
}


void FrontEnd::sendInitialMessages(const char *host, int port) const {
    dbg("FrontEnd.sendInitialMessages",2) << "Sending initial messages to " << host << ":" << port << std::endl;
    char cbuf[10];
    sprintf(cbuf,"%d",port);
    lo_address addr = lo_address_new(host, cbuf);
    lo_send(addr,"/pf/started","");
    sendSetupMessages(host,port);
}

void FrontEnd::sendSetupMessages(const char *host, int port) const {
    dbg("FrontEnd.sendSetupMessages",2) << "Sending setup messages to " << host << ":" << port << std::endl;
    char cbuf[10];
    sprintf(cbuf,"%d",port);
    lo_address addr = lo_address_new(host, cbuf);
    lo_send(addr,"/pf/set/protoversion","s",PROTOVERSION);
    std::string allargs=arglist[0];
    for (unsigned int i=1;i<arglist.size();i++)
	allargs+=" "+arglist[i];
    lo_send(addr,"/pf/set/source","s",allargs.c_str());
    lo_send(addr,"/pf/set/minx","f",world->getMinX()/UNITSPERM);
    lo_send(addr,"/pf/set/maxx","f",world->getMaxX()/UNITSPERM);
    lo_send(addr,"/pf/set/miny","f",world->getMinY()/UNITSPERM);
    lo_send(addr,"/pf/set/maxy","f",world->getMaxY()/UNITSPERM);
    lo_send(addr,"/pf/set/groupdist","f",GROUPDIST/UNITSPERM);
    lo_send(addr,"/pf/set/ungroupdist","f",UNGROUPDIST/UNITSPERM);
    lo_send(addr,"/pf/set/numchannels","i",NCHANNELS);
    lo_send(addr,"/pf/set/fps","f",getFPS()*1.0f);
    if (starttime.tv_sec != 0) {
	//	lo_timetag startTag;
	//startTag.sec=starttime.tv_sec;
	//startTag.frac=(uint32_t)((float)starttime.tv_usec*pow(2.0,32.0)/1e6);
	lo_send(addr,"/pf/set/starttime","ii",starttime.tv_sec,starttime.tv_usec);  // Use ints instead of timetag since dumpOSC doesn't support timetags
    }
}

void FrontEnd::sendUIMessages() {
    if (!touchOSC) 
	return;
    if (frame%50 == 0) {
	lo_send(touchOSC,"/pf/minx","f",world->getMinX()/UNITSPERM);
	lo_send(touchOSC,"/pf/maxx","f",world->getMaxX()/UNITSPERM);
	lo_send(touchOSC,"/pf/miny","f",world->getMinY()/UNITSPERM);
	lo_send(touchOSC,"/pf/maxy","f",world->getMaxY()/UNITSPERM);
	lo_send(touchOSC,"/pf/frame","i",frame);
	lo_send(touchOSC,"/health/FE","f",(frame%100 == 0)?1.0:0.0);
    }
}

void FrontEnd::sendMessages(double elapsed) {
    static bool firsttime=true;
    if  (firsttime) {
	    for (int i=0;i<dests.size();i++)
		sendInitialMessages(dests.getHost(i),dests.getPort(i));
	    firsttime=false;
    }
    sendUIMessages();

    if (frame%200 == 0) {
	// Send setup messages every 200 frames
	for (int i=0;i<dests.size();i++)
	    sendSetupMessages(dests.getHost(i),dests.getPort(i));
    }

    if (frame%2 == 0)
	// Downsample to 25 fps
	world->sendMessages(dests,elapsed);
}

