#pragma once

#include <vector>
#include <string>
#include <assert.h>
#include "lo/lo.h"
#include "point.h"
#include "touchosc.h"

// Class to store each individual relative mapping and associated GUI elements
// These are mappings between 2 or more lasers device space coords that hit the same spot on ground
class RelMapping {
    int id;
    int adjust;
    std::vector<bool>  locked;
    std::vector<Point> devpt;
 public:
    RelMapping(int _id, int nunits): id(_id), adjust(-1), locked(nunits), devpt(nunits) { 
	for (int i=0;i<nunits;i++) {
	    locked[i]=false;
	    devpt[i]=Point(0,0); 
	}
    }
    void updateUI() const;
};

// Mapping from laser device coords to ground level positions
class AbsMapping: public RelMapping {
    int id;
    Point floor;
    bool locked;
 public:
    AbsMapping(int _id, int nunits): RelMapping(-1, nunits), id(_id) { locked=false; }
    void updateUI() const;
};

// Class for handling calibration of laser mappings
// Handles interface to touchOSC calibration GUI
class Calibration {
    static std::shared_ptr<Calibration> theInstance;   // Singleton
    std::vector<std::shared_ptr<RelMapping> > relMappings;		// Relative mappings between lasers
    std::vector<std::shared_ptr<AbsMapping> > absMappings;		// Mapping from laser device coords to ground level positions
    int relSelected;		// Selected relative mapping or -1 for none
    int absSelected;	// Selected abs mapping or -1 for none -- only one of absSelected and relSelected can be active
    bool flipX, flipY;	// to flip display and arrow directions
    float speed;		// speed of arrow movements in device coords/click
    void showStatus(std::string line1,std::string line2="", std::string line3="");	// Display status in touchOSC
    int handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    Calibration(int nunits);   // Only ever called by initialize()
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
};
