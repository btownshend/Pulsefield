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
#include <queue>

#include "point.h"
#include "dbg.h"
#include "background.h"
#include "wrapper.h"


// A frame of data as received from the LIDAR
class SickFrame {
    static Wrapper scanCounterWrapper;
    unsigned int scanCounter;
    int scanCounterWraps;  // Cumulative effect of wrapping scanCounter(adds 1 at each wrap)

    static Wrapper scanTimeWrapper;
    unsigned int scanTime;
    int scanTimeWraps;  // Cumulative effect of wrapping scanTime(adds 1 at each wrap)

    char *readLine(FILE *f) const;
 public:
    static const int MAXECHOES=5;
    static const int MAXMEASUREMENTS=SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS;
    int version;
    int nechoes;
    std::vector<unsigned int> range[MAXECHOES];
    std::vector<unsigned int> reflect[MAXECHOES];
    unsigned int devNumber, serialNumber,devStatus,telegramCounter;
    unsigned int transmitTime,digitalInputs,digitalOutputs;
    float scanFrequency, measurementFrequency;
    unsigned int encoderFlag, encoderPosition, encoderSpeed;
    unsigned int outputChannels;
    unsigned int num_measurements;
    struct timeval acquired;   // Computer time frame was read from network

    // Parameters of sickio itself at time frame was writting to a file
    Point origin;   // Origin in world coordinates
    double coordinateRotation;   // Rotate the [x,y] coordinate system used internally and externally by this many degrees

 public:
    SickFrame();
    void read(SickToolbox::SickLMS5xx *sick_lms_5xx=NULL, int nechoes=1, bool captureRSSI=false, int nMeasure=571);

    // Peek into a file to determine its version
    static int getFileVersion(FILE *fd);
    
    // Read a frame from given fd using given format version
    int  read(FILE *fd, int version=1);

    // Write a frame to given fd, using unit id as given
    void write(FILE *fd, int cid,int version=2) const;

    // Overlay frame read from data file onto real-time data
    void overlayFrame(const SickFrame &frame);


    // Get absolute scan time by adding boottime to (wrapped) scan times
    struct timeval getAbsScanTime(const struct timeval &bootTime) const {
    	    struct timeval result;
	    result.tv_sec=bootTime.tv_sec+scanTimeWraps*4294+scanTime/1000000;
	    result.tv_usec=bootTime.tv_usec+scanTimeWraps*967296+scanTime%1000000;
	    int carry=result.tv_usec/1000000;
	    result.tv_usec-=1000000*carry;
	    result.tv_sec+=carry;
	    return result;
    }

    struct timeval getAquisitionTime() const { return acquired; }
    // Get scan counter (taking into account any wraps since starting this program)
    unsigned int getScanCounter() const {
	return scanCounter + 65536*scanCounterWraps;
    }

    unsigned int getNumEchoes() const {
	return nechoes;
    }
};

class SickIO {
public:
	static const int MAXECHOES=5;
	static const int MAXMEASUREMENTS=SickToolbox::SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS;
	static const int STALEAGE=50000;  // Age after which a frame is considered stale
private:
	std::queue<SickFrame> frames;   // Queue of unprocessed frame
        SickFrame curFrame;  // Current frame
	int id;
	SickToolbox::SickLMS5xx *sick_lms_5xx;
	bool valid;
	struct timeval bootTime;  // Real time equivalent to time 0 on LIDAR (for getAbsScanTime())
	pthread_t runThread;
	int captureEchoes;   // Number of echoes to retrieve in each frame
	bool captureRSSI;
	void get();
	bool running;
	int captureScanFreq;
	double captureScanRes;
	Point origin;   // Origin in world coordinates
	double coordinateRotation;   // Rotate the [x,y] coordinate system used internally and externally by this many degrees
	void updateScanFreqAndRes();
	bool fake;

	// Synchronization
	pthread_mutex_t mutex;  // Lock for accessing the frames queue;  all other accesses shouldn't have threading issues since there is a single producer and single consumer
	pthread_cond_t signal;

	Background bg;		// Background model
	std::vector<Point> calTargets;
	void updateCalTargets();

	// Recording
	static FILE *recordFD;  // Recording FILE, or NULL if disabled
	static const int recordVersion = 2;  // File format version to write
	static pthread_mutex_t recordMutex;  // Shared mutex for keeping output to recording file separated
public:
	SickIO(int _id, const char *host, int port);
	// Constructor to fake the data from a scan
	SickIO(int _id);
	// Push a frame onto queue of frames to be processed
	void pushFrame(const SickFrame &frame);

	// Overlay frame read from data file onto real-time data
	void overlayFrame(const SickFrame &frame);
	virtual ~SickIO();

	void run();
	int start();
	int stop();

	static int startRecording(const char *filename, const std::string &comments) {
	    assert(recordFD==NULL);
	    recordFD = fopen(filename,"w");
	    if (recordFD == NULL) {
		fprintf(stderr,"Unable to open recording file %s for writing\n", filename);
		return -1;
	    }
	    printf("Recording into %s\n", filename);
	    if (recordVersion>1)
		fprintf(recordFD,"FEREC-%d.0\n",recordVersion);
	    fprintf(recordFD,"# %s\n",comments.c_str());   // FIXME: Should check comments for embedded newlines
	    dbg("SickIO.startRecording",1) << "SICK recording to " << filename << " using format version " << recordVersion << " started." << std::endl;
	    return 0;
	}

	static void stopRecording() {
	    (void)fclose(recordFD);
	    recordFD=NULL;
	}
	
	unsigned int getNumMeasurements() const {
		return curFrame.num_measurements;
	}

	const std::vector<Point> getCalTargets() const { return calTargets; }
	const std::vector<unsigned int> getRange(int echo) const {
	    return curFrame.range[echo];
	}

	const std::vector<unsigned int> getReflect(int echo) const {
	    return curFrame.reflect[echo];
	}
	
	// Get time of scan
	struct timeval getAbsScanTime(const SickFrame &f) const {
	    return f.getAbsScanTime(bootTime);
	}
	struct timeval getAbsScanTime() const { return getAbsScanTime(curFrame); }
	// Get acquisition time
	struct timeval getAcquisitionTime() const { return curFrame.getAquisitionTime(); }

	// Get age of frame in usec
	int getAge(const SickFrame &frame) const;
	int getAge() const { return getAge(curFrame); }

	// Get angle of measurement in degrees (local coordinates)
	float getAngleDeg(int measurement)  const {
	    return getScanRes()*(measurement-(curFrame.num_measurements-1)/2.0);
	}
	float getCoordinateRotationDeg() const {
	    return coordinateRotation*180/M_PI;
	}
	float getCoordinateRotationRad() const {
	    return coordinateRotation;
	}
	void setCoordinateRotationDeg(float deg)  {
	    coordinateRotation=M_PI/180*deg;
	}
	void setCoordinateRotationRad(float rad)  {
	    coordinateRotation=rad;
	}
	// Get angle of measurement in radians (local coordinates)
	float getAngleRad(int measurement) const {  
	    return getAngleDeg(measurement)*M_PI/180;
	}
	Point getLocalPoint(int measurement, int echo=0) const {
	    Point p;
	    p.setThetaRange(getAngleRad(measurement), curFrame.range[echo][measurement]);
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
	    dbg("FrontEnd.setOrigin",1) << "Unit " << id << ": Set origin to " << origin << std::endl;
	}

	void setOriginX(float x) {
	    origin.setX(x);
	    dbg("FrontEnd.setOriginX",1) << "Unit " << id << ": Set originX to " << origin << std::endl;
	}

	void setOriginY(float y) {
	    origin.setY(y);
	    dbg("FrontEnd.setOriginY",1) << "Unit " << id << ":Set originY to " << origin << std::endl;
	}
	
	// Distance between two scan points
	float distance(int i, int j) const {
	    Point p1=getLocalPoint(i);
	    Point p2=getLocalPoint(j);
	    return (p1-p2).norm();
	}

	// Get scan counter (taking into account any wraps since starting this program)
	unsigned int getScanCounter() const {
	    return curFrame.getScanCounter();
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

	// Set number of echoes to capture on each frame
	void setCaptureNumEchoes(int nechoes);

	// Get the number of echoes in the current frame (not necessarily the same as the number set for future frames)
	unsigned int getNumEchoes() const { return curFrame.getNumEchoes(); }
	void setSynchronization(bool isMaster, int phase=0);
	void setCaptureRSSI(bool on);
	void setCaptureScanFreq(int freq) {
	    if (freq!=25 && freq!=35 && freq!=50 && freq!=75 && freq!=100) {
		std::cerr << "Illegal scan frequency: " << freq << std::endl;
		exit(1);
	    }
	    captureScanFreq=freq;
	    dbg("SickIO.setScanRes",1) << "Set scan frequency to " << captureScanFreq << " Hz" << std::endl;
	    updateScanFreqAndRes();
	}
	int getScanFreq() const {
	    return curFrame.scanFrequency;
	}
	// Set scan resolution in degrees
	void setCaptureScanRes(double res) {
	    int n=int(190.0/res+1);
	    switch (n) {
	    case 571:
		captureScanRes=0.3333;
		break;
	    case 381:
		captureScanRes=0.5;
		break;
	    default:
		std::cerr << "Illegal scanres: " << res << " (would be " << n << " measurements/scan)" << std:: endl;
		exit(1);
	    }
	    dbg("SickIO.setCaptureScanRes",1) << "Set scan resolution to " << captureScanRes << " degrees" << std::endl;
	    updateScanFreqAndRes();
	}
	// Get scan resolution in degrees
	float getScanRes() const {
	    // get current frame's scanRes
	    return 190.0/(curFrame.num_measurements-1);
	}

	void waitForFrame();

	// Lock/unlock access to the frames queue
	void lock();
	void unlock();

	Background &getBackground()  { return bg; }
	const Background &getBackground() const  { return bg; }
	void updateBackground(const std::vector<int> &assignments, bool all) const {
	    ((SickIO *)this)->bg.update(*this,assignments,all);
	}
	void sendMessages(const char *host, int port) const;
};
