#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
#include <cmath>
#include "lo_util.h"
#include "lo/lo.h"
#include "point.h"
#include "configuration.h"
#undef Status
#include "opencv2/stitching.hpp"
#include "urlconfig.h"

enum UnitType {PROJECTOR, LIDAR, WORLD};

// Class to store each individual relative mapping and associated GUI elements
// These are mappings between 2 or more lasers device space coords that hit the same spot on ground
// Also includes laser->world mapping
class RelMapping {
    const int PTSPERPAIR=10;
    int unit1,unit2;  	// Unit numbers
    UnitType type1, type2;	// Type of units
    std::vector<Point> pt1,pt2;	// Vector of point correspondences
    std::vector<bool> locked;
    int selected; 		// Which point is selected
    float e2sum;		// Sum of sqd error
    int e2cnt;

    int send(std::string path, std::string value) const;
    int send(std::string path, float v1, float v2) const;
    int send(std::string path, float value) const;
public:
    RelMapping(int u1, int u2, UnitType t1, UnitType t2);
    void updateUI(bool flipX1,bool flipY1, bool flipX2, bool flipY2) const;
    bool handleOSCMessage(std::string tok, lo_arg **argv,int argc,float speed,bool flipX1, bool flipY1, bool flipX2, bool flipY2);
    void save(ptree &p) const;
    void load(ptree &p);
    void redistribute();  // Redistribute calibration points optimally
    int addMatches(std::vector<cv::detail::ImageFeatures> &features,    std::vector<cv::detail::MatchesInfo> &pairwiseMatches) const;
    int getUnit(int i) const  { if (i==0) return unit1; else return unit2; }
    void sendCnt() const;  // Send OSC with cnt of locked points to TouchOSC
    Point getDevicePt(int witch,int i=-1,bool doRound=false) const;    // Get coordinate of pt[i] in device or world
    void setDevicePt(Point p, int which,int i=-1);
    std::vector<float> updateErrors();
    std::vector<Point> getCalPoints(int unit,bool selectedOnly) const;
    float getE2Sum() const { return e2sum; }
    int getE2Cnt() const { return e2cnt; }
    void updateTracker() const;
};

// Class for handling calibration of laser mappings
// Handles interface to touchOSC calibration GUI
class Calibration {
    enum LaserMode {CM_NORMAL=0,CM_HOME=1,CM_CURPT=2,CM_CURPAIR=3,CM_ALL=4} laserMode;
    static std::shared_ptr<Calibration> theInstance;   // Singleton
    int nproj;  // Number of projectors
    int nunits;   // Number of projectors+LIDARS (not including world mapping)
    std::vector<std::shared_ptr<RelMapping> > relMappings;		// Relative mappings between [units ,world]
    std::shared_ptr<RelMapping> curMap;	// Currently selected relMapping
    std::vector<bool> flipX, flipY;	// to flip display and arrow directions -- one for each unit+world at end

    float speed;		// speed of arrow movements in device coords/click
    void showStatus(std::string line);	// Display status in touchOSC
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Calibration(int nproj, int nlidar, URLConfig &urls);   // Only ever called by initialize()
    int recompute();		// Use acquired data to estimate transformations
    std::vector<cv::Mat> homographies;		// Each maps from device space to world coordinates (which initially are arbitrary) use w=H*d
    void testMappings() const;
    std::vector<std::string> statusLines;		// Status lines to display
    std::vector<cv::Mat> tvecs, rvecs;			// translation, rotation of camera frame as computed by solvePnP:  note that rotation is done before translation
    std::vector<cv::Mat> poses;				// Position of camera lenses in world space (derived from inverting tvec and rvec)
    std::vector<std::vector<Point>> alignCorners;			// Alignment target positions in real world indexed by LIDAR unit (0..n-1)
    cv::Mat projection;		// Projection matrix for camera (3x3)
    lo_address tosc;

    // Configuration file
    Configuration config;
 public:
    static void initialize(int nproj, int nlidar, URLConfig &urls) { 
	theInstance=std::shared_ptr<Calibration>(new Calibration(nproj, nlidar, urls)); 
	theInstance->load();
	theInstance->updateUI(); 	// After initialization is done
    }
    static std::shared_ptr<Calibration> instance() {
	assert(theInstance);
	return theInstance;
    }
    static int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
	return instance()->handleOSCMessage_impl(path,types,argv,argc,msg);
    }
    void updateUI() const;
    void updateTracker() const;
    void redistribute();  // Redistribute calibration points optimally
    void save();
    void load();
    LaserMode getLaserMode() const { return laserMode; }		// Get the current mode for laser display
    std::vector<Point> getCalPoints(int unit) const;				// Get the set of calibration points that should be drawn for the given laser
    Point map(Point p, int fromUnit, int toUnit=-1) const;		// Map a point in one unit to another (or to the world)
    void setAlignment(int unit, const std::vector<Point> &c) { 
	assert(unit<nproj);
	bool needsUpdate = c.size()!=alignCorners[unit].size();
	alignCorners[unit]=c;
	if (needsUpdate) updateUI();
    }
    std::vector<Point> getAlignment(int unit) const { assert(unit<nproj); return  alignCorners[unit]; }

    int send(std::string path, std::string value) const;
    int send(std::string path, float v1, float v2) const;
    int send(std::string path, float value) const;

    int numProj() const { return nproj; }
    int worldUnit() const { return nunits; }
};
