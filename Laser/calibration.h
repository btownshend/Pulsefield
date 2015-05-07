#pragma once

#include <vector>
#include <string>
#include <assert.h>
#include <cmath>
#include "lo/lo.h"
#include "point.h"
#include "touchosc.h"
#include "configuration.h"
#undef Status
#include "opencv2/stitching/stitcher.hpp"

// Class to store each individual relative mapping and associated GUI elements
// These are mappings between 2 or more lasers device space coords that hit the same spot on ground
// Also includes laser->world mapping
class RelMapping {
    const int PTSPERPAIR=10;
    int unit1,unit2;  	// Unit numbers (world is 1 higher than last laser unit)
    bool isWorld;		// True of unit2 is in world coords
    std::vector<Point> pt1,pt2;	// Vector of point correspondences
    std::vector<bool> locked;
    std::vector<float> error;	// RMS error of each pair; updated by recompute()
    int selected; 		// Which point is selected
public:
    RelMapping(int u1, int u2, bool _isWorld): pt1(PTSPERPAIR), pt2(PTSPERPAIR), locked(PTSPERPAIR), error(PTSPERPAIR) {
	isWorld=_isWorld;
	unit1=u1;
	unit2=u2;
	for (int i=0;i<PTSPERPAIR;i++) {
	    locked[i]=false;
	    pt1[i]=Point(0,0); 
	    if (isWorld)
		pt2[i]=Point(0,2); 
	    else
		pt2[i]=Point(0,0);
	    error[i]=nan("");
	}
    }
    void updateUI() const;
    bool handleOSCMessage(std::string tok, lo_arg **argv,float speed);
    void save(ptree &p) const;
    void load(ptree &p);
    int addMatches(std::vector<cv::detail::ImageFeatures> &features,    std::vector<cv::detail::MatchesInfo> &pairwiseMatches) const;
    int getUnit(int i) { if (i==0) return unit1; else return unit2; }
    void sendCnt() const;  // Send OSC with cnt of locked points to TouchOSC
    Point getDevicePt(int i,int which=-1) const;    // Get coordinate of pt[i] in device [-32767,32767] or world ([-WORLDSIZE,WORLDSIZE],[0,WORLDSIZE])
    void updateErrors(const cv::Mat &H1, const cv::Mat &H2);
};

// Class for handling calibration of laser mappings
// Handles interface to touchOSC calibration GUI
class Calibration {
    static std::shared_ptr<Calibration> theInstance;   // Singleton
    int nunits;
    std::vector<std::shared_ptr<RelMapping> > relMappings;		// Relative mappings between lasers
    std::shared_ptr<RelMapping> curMap;	// Currently selected relMapping
    std::vector<bool> flipX, flipY;	// to flip display and arrow directions -- one for each unit+world at end

    float speed;		// speed of arrow movements in device coords/click
    void showStatus(std::string line1,std::string line2="", std::string line3="") const;	// Display status in touchOSC
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Calibration(int nunits);   // Only ever called by initialize()
    int recompute();		// Use acquired data to estimate transformations
 public:
    static void initialize(int nunits) { assert(!theInstance); theInstance=std::shared_ptr<Calibration>(new Calibration(nunits)); }
    static std::shared_ptr<Calibration> instance() {
	assert(theInstance);
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    void updateUI() const;
    void save(ptree &p) const;
    void load(ptree &p);
};
