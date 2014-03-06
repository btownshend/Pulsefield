/*
 * SickIO.h
 *
 *  Created on: Mar 2, 2014
 *      Author: bst
 */

#ifndef SICKIO_H_
#define SICKIO_H_
#include <pthread.h>
#include <sicklms5xx/SickLMS5xx.hh>

class SickIO {
public:
	static const int MAXECHOES=5;
private:
	int id;
	SickToolbox::SickLMS5xx *sick_lms_5xx;
	unsigned int range[MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
	unsigned int reflect[MAXECHOES][SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
	unsigned int status;
	unsigned int num_measurements;
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
public:
	SickIO(int _id, const char *host, int port);
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

	void setFPS(int fps);
	void setRes(const char *res);
	void setNumEchoes(int nechoes);
	void setCaptureRSSI(bool on);
	void setScanFreq(int freq) {
	    scanFreq=freq;
	    updateScanFreqAndRes();
	}
	void setScanRes(double res) {
	    scanRes=res;
	    updateScanFreqAndRes();
	}
};

#endif /* SICKIO_H_ */
