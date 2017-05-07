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
	setNumEchoes(1);
	setCaptureRSSI(false);
	scanFreq=50;
	scanRes=0.3333;
	coordinateRotation=0;
	origin=Point(0,0);
	updateScanFreqAndRes();
	running=false;
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&signal,NULL);
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


// Overlay data -- must lock before calling
void SickIO::overlayFrame(const SickFrame &frame) {
    assert(valid);
    curFrame.overlayFrame(frame);
    updateCalTargets();
}

void SickIO::updateScanFreqAndRes() {	
    dbg("SickIO.updateScanFreqAndRes",1) << "Updating device to scanFreq=" << scanFreq << "(" << sick_lms_5xx->IntToSickScanFreq(scanFreq) << "), scanRes="
					 << scanRes << "(" << sick_lms_5xx->DoubleToSickScanRes(scanRes) << ")" << std::endl;
    if (!fake)
	    sick_lms_5xx->SetSickScanFreqAndRes(sick_lms_5xx->IntToSickScanFreq(scanFreq),sick_lms_5xx->DoubleToSickScanRes(scanRes));
}

void SickIO::setNumEchoes(int _nechoes) {
    assert(_nechoes>=1 && _nechoes<=MAXECHOES);
    nechoes=_nechoes;
    if (fake)
	return;
    if (nechoes==1)
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
    SickFrame frame(sick_lms_5xx,nechoes,captureRSSI);   // Load from device, blocking if none available yet
    // Adjust bootTime using last frame acquired such that (bootTime+transmitTime ~ acquired)
    if (bootTime.tv_sec==0) {
	// Initialize such that transmitTime is same as acquired Time
	bootTime=frame.acquired;
	bootTime.tv_sec-=frame.transmitTime/1000000;
	bootTime.tv_usec-=frame.transmitTime%1000000;
	if (bootTime.tv_usec<0) {
	    bootTime.tv_sec-=1;
	    bootTime.tv_usec+=1000000;
	}
	dbg("SickIO.get",1) << "Initialized bootTime to " << bootTime.tv_sec << "/" << bootTime.tv_usec << std::endl;
    }
    // compute acquired-bootTime+transmitTime (in usec)
    int error=(frame.acquired.tv_sec-bootTime.tv_sec)*1000000+(frame.acquired.tv_usec-bootTime.tv_usec-frame.transmitTime);
    dbg("SickIO.get",1) << "Unit " << id << " boottime error = " << error << "  usec" << std::endl;
    if (abs(error) > 100000) {
	std::cerr << "Excessive reception delay for unit " << id << " of " << error/1000 << "  msec" << std::endl;
    }
    if (error < 0) {
	dbg("SickIO.get",1) << "Unit " << id << " boottime error = " << error << "  usec, snapping boottime" << std::endl;
	bootTime.tv_sec-=(-error)/1000000;
	bootTime.tv_usec-=(-error)%1000000;
        if (bootTime.tv_usec<0) {
	    bootTime.tv_usec+=1000000;
	    bootTime.tv_sec--;
	}
    }
    if (error>1000) {
	// More than 1msec delay in receiving message
	dbg("SickIO.get",1) << "Receipt of frame " << frame.frame << " from unit " << id << " was delayed by " << error/1000 << " msec." << std::endl;
    }
    static const int DRIFTCOMP=10;   // Can compensate for drift of up to DRIFTCOMP*FPS usec/sec
    if (error > DRIFTCOMP) {
	bootTime.tv_usec-=DRIFTCOMP;
        if (bootTime.tv_usec<0) {
	    bootTime.tv_usec+=1000000;
	    bootTime.tv_sec--;
	}
    }
    lock();
    frames.push(frame);
    if (frames.size() >5) {
	dbg("SickIO.get",1) << "Warning: frames queue now has " << frames.size() << " entries -- flushing " << frames.front().frame <<  std::endl;
	frames.pop();
    }
    pthread_cond_signal(&signal);

    unlock();
}


// Wait until a frame is ready, must be locked before calling
void SickIO::waitForFrame()  {
    if (valid)
	return;  // Already a valid frame present
    while (frames.empty()) {
	dbg("SickIO.waitForFrame",4) << "Waiting for frames" << std::endl;
	pthread_cond_wait(&signal,&mutex);
	dbg("SickIO.waitForFrame",4) << "Cond_wait returned, frames=" << frames.size() << std::endl;
    }
    // Load next frame from queue into curFrame and do any needed processing
    int deltaFrames = frames.front().frame-curFrame.frame;
    if (deltaFrames != 1) {
	dbg("SickIO.get",1) << "Unit " << id << " jumped by " << deltaFrames << " from frame " << curFrame.frame << " to " << frames.front().frame << std::endl;
    }
    
    // Copy in new range data, compute x,y values
    curFrame=frames.front();
    frames.pop();
    updateCalTargets();
    valid=true;

    int dLevel=(getFrame()%100==0)?1:8;
    dbg("SickIO.get",dLevel) << "Frame " << getFrame() << ": got " << curFrame.num_measurements << " measurements" << std::endl;
}

void SickIO::lock() {
    pthread_mutex_lock(&mutex);
}

void SickIO::unlock()  {
    pthread_mutex_unlock(&mutex);
}

void SickIO::updateCalTargets() {
    // Find any calibration targets present
    std::vector<Point> pts(getNumMeasurements());
    for (int i=0;i<getNumMeasurements();i++)  {
	pts[i]=getLocalPoint(i) / UNITSPERM;
    }
    dbg("SickIO.update",3) << "Unit " << id << ": Finding targets from " << pts.size() << " ranges." << std::endl;
    calTargets=findTargets(pts);
}

void SickIO::sendMessages(const char *host, int port) const {
    dbg("SickIO.sendMessages",2) << "Sending  messages to " << host << ":" << port << std::endl;
    char cbuf[10];
    sprintf(cbuf,"%d",port);
    lo_address addr = lo_address_new(host, cbuf);

    // Background
    static int scanpt=0;
    // cycle through all available scanpts to send just four point/transmission, to not load network and keep things balanced
    for (int k=0;k<4;k++) {
	scanpt=(scanpt+1)%bg.getRange(0).size();
	bg.sendMessages(addr,scanpt);
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
