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
#include "background.h"

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
	Point origin;   // Origin in world coordinates
	double coordinateRotation;   // Rotate the [x,y] coordinate system used internally and externally by this many degrees
	void updateScanFreqAndRes();
	bool fake;
	// Synchronization
	pthread_mutex_t mutex;
	pthread_cond_t signal;
	Background bg;		// Background model
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

	// Get angle of measurement in degrees (local coordinates)
	float getAngleDeg(int measurement)  const {
	    return scanRes*(measurement-(num_measurements-1)/2.0);
	}
	float getCoordinateRotationDeg() const {
	    return coordinateRotation*180/M_PI;
	}
	float getCoordinateRotationRad() const {
	    return coordinateRotation;
	}
	void setCoordinateRotationDeg(float deg)  {
	    coordinateRotation=M_PI/180*int(deg/5)*5;  // Round to avoid tiny changes that throw off calibration
	}
	// Get angle of measurement in radians (local coordinates)
	float getAngleRad(int measurement) const {  
	    return getAngleDeg(measurement)*M_PI/180;
	}
	Point getLocalPoint(int measurement, int echo=0) const {
	    Point p;
	    p.setThetaRange(getAngleRad(measurement), range[echo][measurement]);
	    return p;
	}

	Point localToWorld(Point p) const {
	    return p.rotate(coordinateRotation)+origin;
	}
	Point worldToLocal(Point p) const {
	    return (p-origin).rotate(-coordinateRotation);
	}
	
	Point getWorldPoint(int measurement, int echo=0) const {
	    return localToWorld(getLocalPoint(measurement,echo));
	}

	Point getOrigin() const {
	    return origin;
	}

	void setOrigin(Point p)  {
	    origin.setX(p.X());
	    origin.setY(p.Y());
	    dbg("FrontEnd.setOrigin",1) << "Set origin to " << origin << std::endl;
	}

	void setOriginX(float x) {
	    origin.setX(x);
	    dbg("FrontEnd.setOriginX",1) << "Set origin to " << origin << std::endl;
	}

	void setOriginY(float y) {
	    origin.setY(y);
	    dbg("FrontEnd.setOriginY",1) << "Set origin to " << origin << std::endl;
	}
	
	// Distance between two scan points
	float distance(int i, int j) const {
	    Point p1=getLocalPoint(i);
	    Point p2=getLocalPoint(j);
	    return (p1-p2).norm();
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

	Background &getBackground()  { return bg; }
	const Background &getBackground() const  { return bg; }
	void updateBackground(const std::vector<int> &assignments, bool all) const {
	    ((SickIO *)this)->bg.update(*this,assignments,all);
	}
	void sendMessages(const char *host, int port) const;
};
