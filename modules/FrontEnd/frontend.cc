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
#ifdef MATLAB
#include "mat.h"
#endif

int debug=1;

static void error(int num, const char *msg, const char *path)
{
	fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}


FrontEnd::FrontEnd(int _nsick,float maxRange,int argc, const char *argv[]): config("settings.json") {
    dbg("FrontEnd",1) << "FrontEnd::FrontEnd(" << _nsick << ")" << std::endl;

    for (int i=0;i<argc;i++)
	arglist.push_back(argv[i]);

    starttime.tv_sec=0;
    starttime.tv_usec=0;
    
	matframes=0;
	frame = 0;
	nsick=_nsick;
	if (nsick>0)
	    sick = new SickIO*[nsick];
	else
	    sick = 0;
	
	world = new World(maxRange);
	vis = new Vis();
	snap=NULL;  // If needed, set in matsave()
	nechoes=1;
	recordFD=NULL;

	URLConfig urls("config/urlconfig.txt");

	/* Start cameras */
	printf("Initializing with %d sensors...",nsick);fflush(stdout);
	for (int i=0;i<nsick;i++) {
		char ident[20];
		sprintf(ident,"SK%d",i+1);
		int port=urls.getPort(ident);
		if (port<0) {
		    fprintf(stderr,"Unable to locate %s in config file %s\n", ident, urls.getFilename().c_str());
			exit(1);
		}
		const char *host=urls.getHost(ident);
		sick[i]=new SickIO(i+1,host,port);
		sick[i]->start();
	}
	printf("done\n");fflush(stdout);
	load();

	serverPort=urls.getPort("FE");
	if (serverPort<0) {
	    fprintf(stderr,"Invalid server port retrieved from %s when looking for FE: %d\n", urls.getFilename().c_str(),serverPort);
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
	const char *targets[]={"VD","CAL","REC"};
	//const char *targets[]={ "VIS","VD","COND","LAN","REC","COND2","VIS2","LASER"};
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

// Check if LIDARs are frame-shifted from each other; flush a frame from the leader if not
// Return false if something was flushed
bool FrontEnd::syncLIDAR(int i) {
    // Relative to currenttime (last abs scan time processed)
    if (currenttime.tv_sec==0)
	return true;
    int delta=(sick[i]->getAbsScanTime().tv_sec-currenttime.tv_sec)*1000000+(sick[i]->getAbsScanTime().tv_usec-currenttime.tv_usec);
	if (delta/1e6<-1) {
		dbg("FrontEnd.run",1) << "Scan is prior to current time by " << delta/1e6 << " sec; assuming this is a looped playback and resetting current time" << std::endl;
		fprintf(stdout,"Resetting time by %.1f sec (looping playback)\n",delta/1e6);
		currenttime=sick[i]->getAbsScanTime();
	} else if (delta<0)  {
		dbg("FrontEnd.run",1) << "Scan is prior to current time by " << delta << " usec;  flushing scan " << sick[i]->getScanCounter() << " from unit " << i << std::endl;
		sick[i]->clearValid();
		return false;
    }
    return true;
}

void FrontEnd::run() {
    while (!doQuit) {  /* Forever */
	for (int i=0;i<nsick;i++) {
	    while (true) {
		// Wait for next sensor to be valid
		if  (!sick[i]->isValid())
		sick[i]->waitForFrame();
		// Check if its timing is ok
		if (syncLIDAR(i))
		    // LIDARs are in sync
		    break;
		// else get next frame without processing current one
	    }
	    // Read data from sensors	
	    processFrames(i);
	}
    }
}

// Processing incoming OSC messages in a separate thread
void *FrontEnd::processIncoming(void *arg) {
    lo_server s = (lo_server)arg;
    SetDebug("pthread:OSCIncoming");
    dbg("FrontEnd.processIncoming",1) << "Started: s=" << std::setbase(16) << s << std::setbase(10) << std::endl;
    // Process all queued messages
    while (lo_server_recv(s) != 0)
	if (doQuit) {
	    dbg("FrontEnd.processIncoming",0) << "Quit" << std::endl;
	    break;
	}
    return NULL;
}

void FrontEnd::processFrames(int c) {
    int delta=(sick[c]->getAbsScanTime().tv_sec-currenttime.tv_sec)*1000000+(sick[c]->getAbsScanTime().tv_usec-currenttime.tv_usec);
    dbg("FrontEnd.processFrames",2) << "Processing frame " << frame << " using scan " << sick[c]->getScanCounter() << "  from unit " << c+1 << " with delta of " << delta << " usec" << std::endl;
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
	std::vector<unsigned int> range[SickIO::MAXECHOES];
	std::vector<unsigned int> reflect[SickIO::MAXECHOES];
	for (unsigned int i=0;i<sick[c]->getNumEchoes();i++) {
	    range[i]=sick[c]->getRange(i);
	    reflect[i]=sick[c]->getReflect(i);
	}
	sendVisMessages(sick[c]->getId(),sick[c]->getScanCounter(),sick[c]->getAbsScanTime(), sick[c]->getNumMeasurements(), sick[c]->getNumEchoes(), range, reflect);
	double elapsed=0;
	currenttime=sick[c]->getAbsScanTime();
	if (starttime.tv_sec==0)
	    starttime=currenttime;
	dbg("FrontEnd.processFrames",2) << "Processing unit " << sick[c]->getId()  << " scan " << sick[c]->getScanCounter() << " with age " << sick[c]->getAge()/1000.0f << " msec" << std::endl;
	vis->update(sick[c]);
	elapsed=(currenttime.tv_sec-starttime.tv_sec)+(currenttime.tv_usec-starttime.tv_usec)*1e-6;
	world->track(*vis,frame,sick[c]->getScanFreq()*nsick,elapsed);
#ifdef MATLAB
	if (!matfile.empty() && (matframes==0 || frame<matframes))
	    snap->append(frame,vis,world);
#endif
	world->draw(nsick,sick);
	sendMessages(elapsed);
	sendOnce=0;
	dbg("FrontEnd.processFrames",4) << "Bounds=[" << world->getMinX() << "," << world->getMaxX() << "," << world->getMinY() << "," << world->getMaxY() << "]" << std::endl;
	if (tmpDebug) {
	    dbg("FrontEnd.processFrames",1) << "End of frame " << frame << ", restoring debug levels" << std::endl;
	    PopDebugSettings();
	}
	frame++;
    // clear valid flag so another frame can be read
    sick[c]->clearValid();
}

void FrontEnd::sendVisMessages(int id, unsigned int frame, const struct timeval &acquired, int nmeasure, int necho, std::vector<unsigned int> ranges[], std::vector<unsigned int> reflect[]) {
    if (! (sendOnce & (RANGE|REFLECT)))
	return;
    dbg("FrontEnd.sendVisMessages",5) << "sendOnce=0x" << std::setbase(16) << sendOnce << std::setbase(10)  << ", ndest=" << dests.size() << std::endl;

	for (int i=0;i<dests.size();i++) {
		char cbuf[10];
		dbg("FrontEnd.sendVisMessages",6) << "Sending messages to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
		sprintf(cbuf,"%d",dests.getPort(i));
		if (dests.getPort(i)!=7012)	// TODO: this is a hack to send to laser but nobody else; should be configured more generally!
		  continue;
		lo_address addr = lo_address_new(dests.getHost(i), cbuf);
		lo_send(addr,"/vis/beginframe","ii",id,frame);
		if (sendOnce & RANGE) {
		    // Send range info
		    for (int e=0;e<necho;e++) {
			lo_blob data = lo_blob_new(nmeasure*sizeof(*ranges[e].data()),ranges[e].data());
			dbg("FrontEnd.sendVisMessages",6) << "Sending RANGE(" << id << "," << e << ") (N=" << nmeasure << ") to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
			lo_send(addr, "/vis/range","iiiiiib",id,frame,acquired.tv_sec, acquired.tv_usec, e, nmeasure, data);
			lo_blob_free(data);
		    }
		}
		if (sendOnce & REFLECT) {
		    for (int e=0;e<nechoes;e++) {
			lo_blob data = lo_blob_new(nmeasure*sizeof(*reflect[e].data()),reflect[e].data());
			dbg("FrontEnd.sendVisMessages",6) << "Sending REFLECT(" << id << "," << e << ") (N=" << nmeasure << ") to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
			lo_send(addr, "/vis/reflect","iiiiiib",id,frame,acquired.tv_sec, acquired.tv_usec, e, nmeasure, data);
			lo_blob_free(data);
		    }
		}
		lo_send(addr,"/vis/endframe","ii",id,frame);
		lo_address_free(addr);
	}
}

int FrontEnd::playFile(const char *filename,bool singleStep,float speedFactor,bool overlayLive,int frame1, int frameN, bool savePerfData, int maxsick) {
    printf("Playing back recording from %s\n", filename);
    FILE *fd=fopen(filename,"r");
    if (fd == NULL) {
	fprintf(stderr,"Unable to open playback file %s for reading\n", filename);
	return -1;
    }
    struct timeval startfile;
    struct timeval starttime;
    gettimeofday(&starttime,0);
    int frameStep=0;
    int lastscan=-1;
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

    int fileVersion = SickFrame::getFileVersion(fd);
    SickFrame f;

    while (true) {
	// Read the next frame from fd into f and return the ID of the LIDAR 
	int cid = f.read(fd,fileVersion);   
	if (cid<0) {
	    printf("EOF on %s\n",filename);
	    break;
	}
	    
	if (cid>nsick) {
	    if (maxsick>0 && cid>maxsick)
		// Only use part of the data
		continue;
	    printf("Increasing number of LIDARS from %d to %d\n",nsick,cid);
	    SickIO **newSick=new SickIO*[cid];
	    for (int i=0;i<nsick;i++)
		newSick[i]=sick[i];
	    for (int i=nsick;i<cid;i++) {
		newSick[i]=new SickIO(i+1);
	    }
	    if (sick!=0)
		delete [] sick;

	    sick=newSick;
	    removeDefaultHandler();  // To make sure the default is at the end
	    for (int i=nsick;i<cid;i++)
		addSickHandlers(i);	// Add OSC handlers for this new device
	    addDefaultHandler();  // To make sure the default is at the end
	    nsick=cid;
	    load();
	}

	if (lastscan==-1)  {
	    // Initialize file start time for reference
	    startfile=f.acquired;
	}

	lastscan= sick[cid-1]->getScanCounter();
	    
	if  (lastscan==f.getScanCounter()) {
	    fprintf(stderr,"Duplicate scan %d for unit %d\n", f.getScanCounter(), cid);
	    continue;
	}
	
	if (f.getScanCounter()!=lastscan+1 && lastscan!=-1) {
	    if (lastscan+1 == f.getScanCounter()-1) {
		dbg("FrontEnd.playFile",1) << "Input file skips scan " << lastscan+1 << " for unit " << cid << std::endl;
	    } else if (lastscan<f.getScanCounter()) {
		dbg("FrontEnd.playFile",1) << "Input file skips scans " << lastscan+1 << "-" << f.getScanCounter()-1 << " for unit " << cid << std::endl;
	    } else {
		dbg("FrontEnd.playFile",1) << "Input file jumped backwards from scan " << lastscan << " to " << f.getScanCounter() << " for unit " << cid << std::endl;
	    }
	}

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

	    if (speedFactor>0) {
		struct timeval now;
		gettimeofday(&now,0);
	
		long int fileElapsed = (f.acquired.tv_sec-startfile.tv_sec)*1000000 + (f.acquired.tv_usec-startfile.tv_usec);
		long int realElapsed = (now.tv_sec-starttime.tv_sec)*1000000 + (now.tv_usec-starttime.tv_usec);
		long int waittime=(long int)(speedFactor*fileElapsed-realElapsed);
		if (waittime < -1000000) {  // Behind by more than 1 second
		    // Not keeping up;  long and reset starttime
		    std::cerr << "Not keeping up with real time data, jumping clock by 1sec" << std::endl;
		    starttime.tv_sec+=1;
		    waittime+=1000000;
		}
		if (waittime > 100000) {  // >0.1sec
		    std::cerr << "Exessive wait time while playing file: " << waittime << " usec; file elapsed=" << fileElapsed << ", realElapsed=" << realElapsed << std::endl;
		}
		// Keep some slack so we don't delay too much and then get behind
		if (waittime >10000) {
		    // When doing an overlay, timing is driven by file and data is sampled whenever overlay data is ready, if nothing is valid a frame is skipped
		    usleep(waittime-10000);
		}
	    }
	} /* else use real-time timing */

	
	struct timeval processStart; gettimeofday(&processStart,0);

	if (overlayLive) {
	    if (!sick[cid-1]->isValid())
		sick[cid-1]->waitForFrame();
	    sick[cid-1]->overlayFrame(f);
	} else {
	    if (f.version>=2) {
		// Change sick origin, rotation to match values from frame
		if (!(sick[cid-1]->getOrigin() == f.origin)) {
		    std::cout << "Moving origin of unit " << cid << " from " << sick[cid-1]->getOrigin() << " to " << f.origin << std::endl;
		    sick[cid-1]->setOrigin(f.origin);
		}
		if (fabs(sick[cid-1]->getCoordinateRotationRad() -f.coordinateRotation)>1e-4) {
		    std::cout << "Moving rotation of unit " << cid << " from " << sick[cid-1]->getCoordinateRotationRad() << " to " << f.coordinateRotation << " rad" << std::endl;
		    sick[cid-1]->setCoordinateRotationRad(f.coordinateRotation);
		}
	    }
	    sick[cid-1]->pushFrame(f);
	    sick[cid-1]->waitForFrame();  // Won't block since we just pushed a new frame
	}

	// Check if its timing is ok
	if (syncLIDAR(cid-1)) {
	    // LIDARs are in sync
	    if (frameN!=-1 && frame>frameN)
		// Past last frame
		break;
	    if (frame1!=-1 && frame<frame1) {
		// Skip this frame
		frame++;
		continue;   // go to next iteration without saving performance info or matlab snapshots
	    }
	    processFrames(cid-1);
	}  // else, skip this scan

	struct timeval processDone; gettimeofday(&processDone,0);
	float procTime=(processDone.tv_sec-processStart.tv_sec)+(processDone.tv_usec-processStart.tv_usec)/1e6;
	totalProcTime+=procTime;
	totalallProcTime+=procTime;
	maxProcTime=std::max(maxProcTime,procTime);
	nProcTime++;
	nallProcTime++;

	static int lastStatsFrame=0;
	if (frame-lastStatsFrame >=200) {
	    dbg("frontend",1) << "Frame " << frame << ": mean frame processing time= " <<  totalProcTime/nProcTime << ", max=" << maxProcTime << ", max FPS=" << nProcTime/totalProcTime << std::endl;
	    printf("Playing frame %d with mean processing time=%.1f ms (%.0f FPS), max=%.1f ms\n",frame,totalProcTime/nProcTime*1000,nProcTime/totalProcTime,maxProcTime*1000);
	    nProcTime=0;
	    maxProcTime=0;
	    totalProcTime=0;
	    lastStatsFrame=frame;
	}

#ifdef MATLAB
	if (!matfile.empty()) {
	    if (matframes>0 && frame>=matframes)
		// Do final output below
		break;

	    if (snap->size() >= 2000) {
		// Break up the output
		char tmpfile[1000];
		sprintf(tmpfile,"%s-%d.mat",matfile.c_str(),frame);
		snap->save(tmpfile);
		snap->clear();
	    }
	}
#endif
	minPerfFrame=std::min(minPerfFrame,(int)(f.getScanCounter()-1));
	maxPerfFrame=std::max(maxPerfFrame,(int)(f.getScanCounter()-1));
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
	} else if (allperf.size()==0) {
	    fprintf(stderr,"No performance data to write (no people?)\n");
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

#ifdef MATLAB
    if (!matfile.empty()) {
	char tmpfile[1000];
	sprintf(tmpfile,"%s-%d.mat",matfile.c_str(),frame);
	snap->save(tmpfile);
	snap->clear();
    }
#endif
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
    // TODO: Send data for multiple LIDARs
    lo_send(addr,"/pf/set/rotation","f",sick[0]->getCoordinateRotationDeg());
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
	for (int i=0;i<nsick;i++) {
	    char msg[100];
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/rotation").c_str(),"f",sick[i]->getCoordinateRotationDeg());
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/x").c_str(),"f",sick[i]->getOrigin().X()/UNITSPERM);
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/y").c_str(),"f",sick[i]->getOrigin().Y()/UNITSPERM);
	    sprintf(msg,"Rotation: %.0f",sick[i]->getCoordinateRotationDeg());
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/rotation/label").c_str(),"s",msg);
	    sprintf(msg,"X: %.2f",sick[i]->getOrigin().X()/UNITSPERM);
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/x/label").c_str(),"s",msg);
	    sprintf(msg,"Y: %.2f",sick[i]->getOrigin().Y()/UNITSPERM);
	    lo_send(touchOSC,("/pf/sick"+std::to_string(i)+"/y/label").c_str(),"s",msg);
	}
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

    static int lastFrame=0;
    if (frame != lastFrame) {
	// Downsample to 25 fps
	world->sendMessages(dests,elapsed);
	for (int j=0;j<nsick;j++)
	    for (int i=0;i<dests.size();i++)
		sick[j]->sendMessages(dests.getHost(i),dests.getPort(i));
	lastFrame=frame;
    }

    if (frame%10 == 0) {
	// Send outsider messages every 10 frames
	for (int i=0;i<dests.size();i++)
	    world->getOutsiders().sendMessages(dests.getHost(i),dests.getPort(i));
    }
}

void FrontEnd::save()  {
    dbg("FrontEnd.save",1) << "Saving settings" << std::endl;
    ptree p;
    p.put("minx",world->getMinX());
    p.put("maxx",world->getMaxX());
    p.put("miny",world->getMinY());
    p.put("maxy",world->getMaxY());
    config.pt().put_child("world",p);
    for (int i=0;i<nsick;i++) {
	std::string path="sick"+std::to_string(i);
	ptree p;
	p.put("rotation",sick[i]->getCoordinateRotationDeg());
	p.put("origin.x",sick[i]->getOrigin().X());
	p.put("origin.y",sick[i]->getOrigin().Y());
	config.pt().put_child(path,p);
    }
    config.save();
}

void FrontEnd::load() {
    dbg("FrontEnd.load",1) << "Loading settings" << std::endl;

    config.load();
    ptree &p1=config.pt();
    ptree p;
    try {
	p=p1.get_child("world");
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Unable to find 'world' in settings file" << std::endl;
	return;
    }
    assert(world);
    world->setMinX(p.get("minx",-1.0f));
    world->setMaxX(p.get("maxx",1.0f));
    world->setMinY(p.get("miny",0.0f));
    world->setMaxY(p.get("maxy",1.0f));
    for (int i=0;i<nsick;i++) {
	assert(sick && sick[i]);
	std::string path="sick"+std::to_string(i)+".";
	sick[i]->setCoordinateRotationDeg(p1.get(path+"rotation",0.0f));
	sick[i]->setOrigin(Point(p1.get(path+"origin.x",0.0f),p1.get(path+"origin.y",0.0f)));
    }
}
