#pragma once

#include "dbg.h"
#include "urlconfig.h"
#include "opencv2/stitching.hpp"
#include "point.h"

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
    lo_address remote,frontend;
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

    // Send various parts of the transformation between world and projector
    void sendMatrix(const char *path, int unit, const cv::Mat &mat) const;

    // Send complete transforms to go between world coordinates (in meters) and screen coordinates (0-width,0-height)
    // Note that these are 3x3 homogenous matrices
    void sendWorldToScreen(int unit, const cv::Mat &mat) const { sendMatrix("/cal/world2screen", unit, mat); }
    void sendScreenToWorld(int unit, const cv::Mat &mat) const { sendMatrix("/cal/screen2world", unit, mat); }
    
    // Send position of projector; although this may be redundant with the other information, calculating it is non-trivial
    void sendPose(int unit, const cv::Mat &pose) const;

    // Send position and orientation of LIDAR
    void sendLIDARPose(int unit, const cv::Mat &homography) const ;
    
    // Send projection (perspective map)
    void sendProjection(int unit, const cv::Mat &projection) const;
    
    // Send transform that converts world coordinates into the camera view frame (without projecting)
    // Consists of [R:T] where R is 3x3 rotation matrix and T is 3x1 translation
    // Translation is applied after rotation
    void sendCameraView(int unit, const cv::Mat &rotMat, const cv::Mat &tvec) const;

    void sendMapping(int unit1, int unit2, int pnum, int numPoints, Point p1, Point p2) const;
};
