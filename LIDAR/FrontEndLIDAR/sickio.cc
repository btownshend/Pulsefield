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

#include "mat.h"
#include "dbg.h"

using namespace SickToolbox;
using namespace std;

static void *runner(void *t) {
	((SickIO*)t)->run();
	return NULL;
}

SickIO::SickIO(int _id, const char *host, int port) {
	/*
	 * Initialize the Sick LMS 2xx
	 */
	id=_id;
	frame=0;
	valid=false;
	num_measurements=0;
	status=0;
	fake=false;

	if (!fake)
		try {
			sick_lms_5xx = new SickLMS5xx(host,port);
			sick_lms_5xx->Initialize();
		} catch(...) {
			fprintf(stderr,"Initialize failed! Are you using the correct IP address?\n");
			exit(1);
		}
	setNumEchoes(1);
	setCaptureRSSI(false);
	scanFreq=50;
	scanRes=0.5;
	updateScanFreqAndRes();
	running=false;
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

void SickIO::set(int _id, int _frame, const timeval &_acquired, int _nmeasure, int _nechoes, unsigned int _range[][MAXMEASUREMENTS], unsigned int _reflect[][MAXMEASUREMENTS]){
    id=_id;
    frame=_frame;
    acquired=_acquired;
    num_measurements=_nmeasure;
    nechoes=_nechoes;
    scanRes=190.0/(num_measurements-1);

    // Copy in data
    for (int e=0;e<nechoes;e++)
	for (int i=0;i<num_measurements;i++) {
	    range[e][i]=_range[e][i];
	    reflect[e][i]=_reflect[e][i];
	    x[e][i]=cos(getAngleRad(i)+M_PI/2)*range[e][i];
	    y[e][i]=sin(getAngleRad(i)+M_PI/2)*range[e][i];
	}
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

void SickIO::get() {
	try {
		//unsigned int range_2_vals[SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
		//sick_lms_5xx.SetSickScanFreqAndRes(SickLMS5xx::SICK_LMS_5XX_SCAN_FREQ_25,
		//SickLMS5xx::SICK_LMS_5XX_SCAN_RES_25);
		//sick_lms_5xx.SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_DIST_DOUBLE_PULSE,
		//				         SickLMS5xx::SICK_LMS_5XX_REFLECT_NONE);
		assert(nechoes>=1 && nechoes<=MAXECHOES);
		if (fake) {
			num_measurements=190/scanRes+1;
			for (int i=0;i<(int)num_measurements;i++) {
				for (int e=0;e<nechoes;e++) {
					range[e][i]=i+e*100;
					reflect[e][i]=100/(e+1);
				}
			}
			status=1;
			usleep(1000000/scanFreq);
		} else
			sick_lms_5xx->GetSickMeasurements(
				range[0], (nechoes>=2)?range[1]:NULL, (nechoes>=3)?range[2]:NULL, (nechoes>=4)?range[3]:NULL, (nechoes>=5)?range[4]:NULL,
				captureRSSI?reflect[0]:NULL, (captureRSSI&&nechoes>=2)?reflect[1]:NULL, (captureRSSI&&nechoes>=3)?reflect[2]:NULL, (captureRSSI&&nechoes>=4)?reflect[3]:NULL, (captureRSSI&&nechoes>=5)?reflect[4]:NULL,
				*((unsigned int *)&num_measurements),&status);
		
		// Compute x,y values
		for (int i=0;i<num_measurements;i++)
		    for (int e=0;e<nechoes;e++) {
			x[e][i]=cos(getAngleRad(i)+M_PI/2)*range[e][i];
			y[e][i]=sin(getAngleRad(i)+M_PI/2)*range[e][i];
		    }
	}

	catch(const SickConfigException & sick_exception) {
		printf("%s\n",sick_exception.what());
	}

	catch(const SickIOException & sick_exception) {
		printf("%s\n",sick_exception.what());
	}

	catch(const SickTimeoutException & sick_exception) {
		printf("%s\n",sick_exception.what());
	}

	catch(...) {
		fprintf(stderr,"An Error Occurred!\n");
		throw;
	}

	gettimeofday(&acquired,0);
	if (valid)
	    fprintf(stderr,"Warning, frame %d overwritten before being retrieved\n", frame);
	frame++;
	valid=true;
	if (frame%100==0)
	    dbg("SickIO.get",1) << "Frame " << frame << ": got " << num_measurements << " measurements, status=" << status << std::endl;
}

