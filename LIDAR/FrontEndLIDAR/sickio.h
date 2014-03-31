/*
 * SickIO.h
 *
 *  Created on: Mar 2, 2014
 *      Author: bst
 */

#ifndef SICKIO_H_
#define SICKIO_H_
#include <math.h>
#include <pthread.h>
#include <sicklms5xx/SickLMS5xx.hh>
#include <mat.h>

#include "point.h"

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
	unsigned int frame;
	bool valid;
	pthread_t runThread;
	int nechoes;
	bool captureRSSI;
	void get();
	bool running;
	int scanFreq;
	double scanRes;
	void updateScanFreqAndRes();
	bool fake;
public:
	SickIO(int _id, const char *host, int port);
	// Constructor to fake the data from a scan
	SickIO(int _id, int _frame, const timeval &_acquired, int _nmeasure, int _nechoes, unsigned int _range[][MAXMEASUREMENTS], unsigned int _reflect[][MAXMEASUREMENTS]){
	    fake=true;
	    id=_id;
	    frame=_frame;
	    acquired=_acquired;
	    num_measurements=_nmeasure;
	    nechoes=_nechoes;
	    for (int i=0;i<nechoes;i++)
		for (int j=0;j<num_measurements;j++) {
		    range[i][j]=_range[i][j];
		    reflect[i][j]=_reflect[i][j];
		}
	    scanRes=190.0/(num_measurements-1);
	    scanFreq=50;
	    updateScanFreqAndRes();
	}

	virtual ~SickIO();

	void run();
	int start();
	int stop();

	int getNumMeasurements() const {
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
	float getAngle(int measurement)  const {
	    return scanRes*(measurement-(num_measurements-1)/2.0);
	}

	float getX(int measurement, int echo=0)  const {
	    return cos(getAngle(measurement)*M_PI/180+M_PI/2)*range[echo][measurement];
	}

	float getY(int measurement, int echo=0) const {
	    return sin(getAngle(measurement)*M_PI/180+M_PI/2)*range[echo][measurement];
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
		this->valid = false;
	}

	int getId() const {
		return id;
	}

	void setNumEchoes(int nechoes);
	int getNumEchoes() const { return nechoes; }
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
};

#endif /* SICKIO_H_ */
