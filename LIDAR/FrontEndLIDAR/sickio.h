/*
 * SickIO.h
 *
 *  Created on: Mar 2, 2014
 *      Author: bst
 */

#pragma once

#include <math.h>
#include <pthread.h>
#include <sicklms5xx/SickLMS5xx.hh>
#include <mat.h>

#include "point.h"
#include "dbg.h"

class SickIO {
public:
	static const int MAXECHOES=5;
	static const int MAXMEASUREMENTS=SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS;
private:
	int id;
	SickToolbox::SickLMS5xx *sick_lms_5xx;
	unsigned int range[MAXECHOES][MAXMEASUREMENTS];
	unsigned int reflect[MAXECHOES][MAXMEASUREMENTS];
	unsigned int status;
	// Compute x,y based on acquired ranges
	float x[MAXECHOES][MAXMEASUREMENTS];
	float y[MAXECHOES][MAXMEASUREMENTS];
	int num_measurements;
	struct timeval acquired;
	unsigned int frame;   // Frame number of frame currently stored in range,reflect,x,y,acquired
	unsigned int frameCntr;  // Increments at each frame time (even if a frame is overwritten)
	unsigned int overwrittenframes;  // Number of overwritten frames since last message
	bool valid;
	pthread_t runThread;
	int nechoes;
	bool captureRSSI;
	void get();
	bool running;
	int scanFreq;
	double scanRes;
	double coordinateRotation;   // Rotate the [x,y] coordinate system used internally and externally by this many degrees
	void updateScanFreqAndRes();
	bool fake;
	// Synchronization
	pthread_mutex_t mutex;
	pthread_cond_t signal;
public:
	SickIO(int _id, const char *host, int port);
	// Constructor to fake the data from a scan
	SickIO() {
	    fake=true;
	    scanFreq=50;
	    frame=0;
	    frameCntr=1;
	    valid=false;
	    pthread_mutex_init(&mutex,NULL);
	    pthread_cond_init(&signal,NULL);
	    coordinateRotation=0;
	}
	// Set values for faking
	void set(int _id, int _frame, const timeval &_acquired, int _nmeasure, int _nechoes, unsigned int _range[][MAXMEASUREMENTS], unsigned int _reflect[][MAXMEASUREMENTS]);

	// Overlay frame read from data file onto real-time data
	void overlay(int _id, int _frame, const timeval &_acquired, int _nmeasure, int _nechoes, unsigned int _range[][MAXMEASUREMENTS], unsigned int _reflect[][MAXMEASUREMENTS]);
	virtual ~SickIO();

	void run();
	int start();
	int stop();

	unsigned int getNumMeasurements() const {
		return num_measurements;
	}

	const unsigned int* getRange(int echo) const {
	    return range[echo];
	}

	const unsigned int* getReflect(int echo) const {
	    return reflect[echo];
	}

	const struct timeval& getAcquired() const {
		return acquired;
	}

	// Get angle of measurement in degrees
	float getAngleDeg(int measurement)  const {
	    return scanRes*(measurement-(num_measurements-1)/2.0) +coordinateRotation;
	}
	float getCoordinateRotationDeg() const {
	    return coordinateRotation;
	}
	void setCoordinateRotationDeg(float deg)  {
	    coordinateRotation=int(deg/5)*5;  // Round to avoid tiny changes that throw off calibration
	}
	float getAngleRad(int measurement) const {
	    return getAngleDeg(measurement)*M_PI/180;
	}

	float getX(int measurement, int echo=0)  const {
	    return x[echo][measurement];
	}

	float getY(int measurement, int echo=0) const {
	    return y[echo][measurement];
	}

	Point getPoint(int measurement) const {
	    return Point(getX(measurement),getY(measurement));
	}
	
	// Get scan index closest to angle (in degrees) 
	int getScanAtAngle(float angle) const {
	    while (angle>180)
		angle-=2*180;
	    while (angle<-180)
		angle+=2*180;
	    return (int)(angle/scanRes+(num_measurements-1)/2.0 + 0.5);
	}

	// Distance between two scan points
	float distance(int i, int j) const {
	    return sqrt(pow(getX(i)-getX(j),2) + pow(getY(i)-getY(j),2));
	}

	unsigned int getFrame() const {
		return frame;
	}

	bool isValid() const {
		return valid;
	}

	void clearValid() {
	    dbg("SickIO.clearValid",8) << "Cleared" << std::endl;
		this->valid = false;
	}

	int getId() const {
		return id;
	}

	void setNumEchoes(int nechoes);
	unsigned int getNumEchoes() const { return nechoes; }
	void setCaptureRSSI(bool on);
	void setScanFreq(int freq) {
	    scanFreq=freq;
	    updateScanFreqAndRes();
	}
	int getScanFreq() const {
	    return scanFreq;
	}
	// Set scan resolution in degrees
	void setScanRes(double res) {
	    scanRes=res;
	    updateScanFreqAndRes();
	}
	// Get scan resolution in degrees
	float getScanRes() const {
	    return scanRes;
	}

	void waitForFrame();
	void lock();
	void unlock();
};
