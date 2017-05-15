/*
 * SickIO.cc
 *
 *  Created on: Mar 2, 2014
 *      Author: bst
 */

#include <math.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <pthread.h>
#include "sickio.h"
#include "findtargets.h"
#include "parameters.h"
#include "mat.h"
#include "dbg.h"

using namespace SickToolbox;
using namespace std;

pthread_mutex_t SickIO::recordMutex=PTHREAD_MUTEX_INITIALIZER;  // Shared mutex for keeping output to recording file separated
FILE *SickIO::recordFD=NULL;

static void *runner(void *t) {
    SetDebug("pthread:SickIO");
	((SickIO*)t)->run();
	return NULL;
}

SickIO::SickIO(int _id, const char *host, int port) {
	/*
	 * Initialize the Sick LMS 2xx
	 */
	id=_id;
	valid=false;
	fake=false;

	if (!fake)
		try {
			sick_lms_5xx = new SickLMS5xx(host,port);
			sick_lms_5xx->Initialize();
		} catch(...) {
			fprintf(stderr,"Initialize failed! Are you using the correct IP address?\n");
			exit(1);
		}
	if (id==1)
	    setSynchronization(true);
	else
	    setSynchronization(false,180);
	setCaptureNumEchoes(1);
	setCaptureRSSI(false);
	captureScanFreq=50;
	captureScanRes=0.3333;   // Needs to be only 4 decimals to be recognized by DoubleToSickScanRes
	updateScanFreqAndRes();
	coordinateRotation=0;
	origin=Point(0,0);
	running=false;
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&signal,NULL);
}

SickIO::SickIO(int _id) {
    id=_id;
    fake=true;
    captureScanFreq=50;
    captureScanRes=0.3333;
    updateScanFreqAndRes();
    coordinateRotation=0;
    origin=Point(0,0);
    valid=false;
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&signal,NULL);
    coordinateRotation=0;
}

SickIO::~SickIO() {
	if (fake)
		return;

	try {
		sick_lms_5xx->Uninitialize();
	} catch(...) {
		fprintf(stderr,"Uninitialize failed!\n");
	}
}


// Overlay given frame over current frame keeping nearest points
void SickIO::overlayFrame(const SickFrame &frame) {
    assert(valid);
    curFrame.overlayFrame(frame);
    updateCalTargets();
}

void SickIO::updateScanFreqAndRes() {	
    dbg("SickIO.updateScanFreqAndRes",1) << "Updating device to scanFreq=" << captureScanFreq << "(" << sick_lms_5xx->IntToSickScanFreq(captureScanFreq) << "), scanRes="
					 << captureScanRes << "(" << sick_lms_5xx->DoubleToSickScanRes(captureScanRes) << ")" << std::endl;
    if (!fake)
	    sick_lms_5xx->SetSickScanFreqAndRes(sick_lms_5xx->IntToSickScanFreq(captureScanFreq),sick_lms_5xx->DoubleToSickScanRes(captureScanRes));
}

void SickIO::setCaptureNumEchoes(int n) {
    assert(n>=1 && n<=MAXECHOES);
    captureEchoes=n;
    if (fake)
	return;
    if (captureEchoes==1)
	sick_lms_5xx->SetSickEchoFilter(SickLMS5xx::SICK_LMS_5XX_ECHO_FILTER_FIRST);
    else
	sick_lms_5xx->SetSickEchoFilter(SickLMS5xx::SICK_LMS_5XX_ECHO_FILTER_ALL_ECHOES);
}

void SickIO::setSynchronization(bool isMaster, int phase) {
    if (isMaster) {
	sick_lms_5xx->SetSickOutput6Mode(SickLMS5xx::SICK_LMS_5XX_OUTPUT_MODE_MASTER_SYNC);
    } else {
	sick_lms_5xx->SetSickInput3Mode(SickLMS5xx::SICK_LMS_5XX_INPUT_MODE_SLAVE_SYNC);
	sick_lms_5xx->SetSickSyncPhase(phase);
    }
}

void SickIO::setCaptureRSSI(bool on) {
    captureRSSI=on;
    if (fake)
	return;

    if (captureRSSI)
	sick_lms_5xx->SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_SCAN_FORMAT_DIST_REFLECT);
    else
	sick_lms_5xx->SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_SCAN_FORMAT_DIST);
}


int SickIO::start() {
    if (running)
	return 0;
    int rc=pthread_create(&runThread, NULL, runner, (void *)this);
    if (rc) {
	fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	return -1;
    }
    running=true;
    return 0;
}

int SickIO::stop() {
    if (!running)
	return 0;
    // Stop
    fprintf(stderr,"SickIO::startStop canceling thread\n");
    int rc=pthread_cancel(runThread);
    if (rc) {
	fprintf(stderr,"pthread_cancel failed with error code %d\n", rc);
	return -1;
    }
    running=false;
    return 0;
}

void SickIO::run() {
    dbg("SickIO.run",1) << "Running" << std::endl;
    while (true)
	get();
}

// Background thread that retrieves frames from device and stores them in a queue
void SickIO::get() {
    SickFrame frame;
    frame.read(sick_lms_5xx,captureEchoes,captureRSSI);   // Load from device, blocking if none available yet
    pushFrame(frame);
}

// Push a frame onto the queue of incoming frames
void SickIO::pushFrame(const SickFrame &frame) {
    // Adjust bootTime using last frame acquired such that bootTime+scanTime ~ acquired
    if (bootTime.tv_sec==0) {
	// Initialize such that transmitTime is same as acquired Time
	struct timeval relScanTime=frame.getAbsScanTime(bootTime);   // bootTime=0, so this is relative
	bootTime=frame.acquired;
	bootTime.tv_sec-=relScanTime.tv_sec;
	bootTime.tv_usec-=relScanTime.tv_usec;
	if (bootTime.tv_usec<0) {
	    bootTime.tv_sec-=1;
	    bootTime.tv_usec+=1000000;
	}
	assert(bootTime.tv_usec < 1000000);
	assert(bootTime.tv_sec>0);
	dbg("SickIO.get",1) << "Unit " << id << " initialized bootTime to " << bootTime.tv_sec << "/" << bootTime.tv_usec << std::endl;
    }

    // compute acquired-absScanTime
    struct timeval absScanTime = frame.getAbsScanTime(bootTime);
    int error=(frame.acquired.tv_sec-absScanTime.tv_sec)*1000000+(frame.acquired.tv_usec-absScanTime.tv_usec);
    dbg("SickIO.get",2) << "Unit " << id << " scan " << frame.getScanCounter() << " received  " << error << "  usec after scan started." << std::endl;
    if (error < 0) {
	bootTime.tv_sec-=(-error)/1000000;
	bootTime.tv_usec-=(-error)%1000000;
        if (bootTime.tv_usec<0) {
	    bootTime.tv_usec+=1000000;
	    bootTime.tv_sec--;
	}
	dbg("SickIO.get",1) << "Unit " << id << " boottime error = " << error << "  usec, snapping boottime to " << bootTime.tv_sec << "/" << bootTime.tv_usec  << std::endl;
    }
    if (error>=5000) {
	// More than 5msec delay in receiving message
	dbg("SickIO.get",1) << "Receipt of scan " << frame.getScanCounter() << " from unit " << id << " was delayed by " << error/1000 << " msec." << std::endl;
	if (error > 100000)
	    std::cerr << "Excessive reception delay for unit " << id << " of " << error/1000 << "  msec" << std::endl;
    }

    static const int DRIFTCOMP=10;   // Can compensate for drift of up to DRIFTCOMP*FPS usec/sec
    if (error > DRIFTCOMP) {
	bootTime.tv_usec+=DRIFTCOMP;
        if (bootTime.tv_usec<0) {
	    bootTime.tv_usec+=1000000;
	    bootTime.tv_sec--;
	}
    }
    lock();
    frames.push(frame);
    while (frames.size()>1 && getAge(frames.front())>STALEAGE) {
	dbg("SickIO.get",1) << "Discarding stale scan " << frames.front().getScanCounter() << " from unit " << id << " with age " << getAge(frames.front()) << std::endl;
	dbg("SickIO.get",3) << "scan Time = " << frames.front().getAbsScanTime(bootTime).tv_sec << "/" << frames.front().getAbsScanTime(bootTime).tv_usec << std::endl;
	frames.pop();
    }
    if (frames.size() >5) {
	dbg("SickIO.get",1) << "Warning: scan queue for unit " << id << " now has " << frames.size() << " entries -- flushing " << frames.front().getScanCounter() <<  std::endl;
	frames.pop();
    }
    pthread_cond_signal(&signal);
    unlock();
    if (recordFD != NULL) {
	pthread_mutex_lock(&recordMutex);   /// Make sure only one thread at a time writes a frame
	dbg("SickIO.write",2) << "Unit " << id << " writing scan " << frame.getScanCounter() << std::endl;
	if (recordVersion>=2)
	    fprintf(recordFD,"T %d %f %f %f\n",id, origin.X(), origin.Y(), coordinateRotation);
	frame.write(recordFD, id, recordVersion);
	dbg("SickIO.write",2) << "Unit " << id << " done writing scan " << frame.getScanCounter() << std::endl;
	pthread_mutex_unlock(&recordMutex);
    }
}


// Wait until a frame is ready
void SickIO::waitForFrame()  {
    if (valid)
	return;  // Already a valid frame present
    while (frames.empty()) {
	assert(!fake);   // Should never end up here when faking
	dbg("SickIO.waitForFrame",4) << "Waiting for next scan" << std::endl;
	lock();  // Need to lock here to not miss a signal
	if (frames.empty())
	    // Still empty, but now we have a lock so we won't miss a signal
	    pthread_cond_wait(&signal,&mutex);
	unlock();
	dbg("SickIO.waitForFrame",4) << "Cond_wait returned, have " << frames.size() << " scans" << std::endl;
    }
    // Load next frame from queue into curFrame and do any needed processing
    int deltaFrames = frames.front().getScanCounter()-curFrame.getScanCounter();
    if (deltaFrames != 1) {
	dbg("SickIO.get",1) << "Unit " << id << " jumped by " << deltaFrames << " from scan  " << curFrame.getScanCounter() << " to " << frames.front().getScanCounter() << std::endl;
    }
    
    // Copy in new range data, compute x,y values
    lock();
    curFrame=frames.front();
    frames.pop();
    unlock();
    
    updateCalTargets();
    valid=true;

    int dLevel=(getScanCounter()%100==0)?1:8;
    dbg("SickIO.get",dLevel) << "Scan " << getScanCounter() << ": got " << curFrame.num_measurements << " measurements" << std::endl;
}

void SickIO::lock() {
    pthread_mutex_lock(&mutex);
}

void SickIO::unlock()  {
    pthread_mutex_unlock(&mutex);
}

// Get age of frame in usec
int SickIO::getAge(const SickFrame &f) const {
    struct timeval scanTime = getAbsScanTime(f);
    int age=(f.acquired.tv_sec-scanTime.tv_sec)*1000000+(f.acquired.tv_usec-scanTime.tv_usec);
    return age;
}
	

void SickIO::updateCalTargets() {
    // Find any calibration targets present
    std::vector<Point> pts(getNumMeasurements());
    for (int i=0;i<getNumMeasurements();i++)  {
	pts[i]=getLocalPoint(i) / UNITSPERM;
    }
    dbg("SickIO.update",3) << "Unit " << id << ": Finding targets from " << pts.size() << " ranges." << std::endl;
    std::vector<Point> newCalTargets=findTargets(pts);
    // Refine use prior ones
    for (int i=0;i<newCalTargets.size();i++)
	for (int j=0;j<calTargets.size();j++) {
	    if ( (calTargets[j]-newCalTargets[i]).norm() < 0.01) { // Within 1 cm
		dbg("SickIO.updateCalTargets",1) << "Unit " << id << ": Smoothing " << newCalTargets[j] << " using prior target " << calTargets[j] << std::endl;
		static const float OLDWEIGHT=0.95;
		newCalTargets[i] = newCalTargets[i]*(1-OLDWEIGHT) + calTargets[j]*OLDWEIGHT;
	    }
	}
    calTargets=newCalTargets;
}

void SickIO::sendMessages(const char *host, int port) const {
    dbg("SickIO.sendMessages",2) << "Sending  messages to " << host << ":" << port << std::endl;
    char cbuf[10];
    sprintf(cbuf,"%d",port);
    lo_address addr = lo_address_new(host, cbuf);

    // Background
    if (bg.getRange(0).size() > 0) {
	static int scanpt=0;
	// cycle through all available scanpts to send just four point/transmission, to not load network and keep things balanced
	for (int k=0;k<4;k++) {
	    scanpt=(scanpt+1)%bg.getRange(0).size();
	    bg.sendMessages(addr,scanpt);
	}
    }

    // Send Calibration Targets
    for (int i=0;i<calTargets.size();i++) {
	Point w=localToWorld(calTargets[i]*UNITSPERM)/UNITSPERM;
	lo_send(addr,"/pf/aligncorner","iiiffff",id,i,calTargets.size(),calTargets[i].X(),calTargets[i].Y(),w.X(),w.Y());
    }
    if (calTargets.size()==0) 
	lo_send(addr,"/pf/aligncorner","iiiffff",id,-1,calTargets.size(),0.0,0.0,0.0,0.0);  // So receiver can clear list

    // Done!
    lo_address_free(addr);
}
