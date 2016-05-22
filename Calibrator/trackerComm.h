#pragma once

#include "dbg.h"

// Cursors to be displayed in Tracker
class Cursor {
 public:
    int unit;
    float x,y;
    Cursor(int unit, float x, float y) { this->x=x; this->y=y; this->unit=unit; }
};

// Class for communication with java Tracker program
class TrackerComm {
    static TrackerComm *theInstance;   // Singleton
    lo_address remote;
    TrackerComm(URLConfig &urls);
    ~TrackerComm();
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
 public:
    static void initialize(URLConfig &urls) {
	assert(theInstance == NULL);
	theInstance=new TrackerComm(urls);
    }
    static TrackerComm *instance() {
	assert(theInstance != NULL);
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    void sendCursors(const std::vector<Cursor> &c) const;
    void sendTransform(int unit, const std::vector<float> &data) const;
};
