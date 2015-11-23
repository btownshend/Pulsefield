#pragma once

#include <vector>
#include "etherdream_bst.h"
#include "displaydevice.h"
#include "color.h"
#include "touchosc.h"

class Drawing;
class Ranges;

class Laser: public DisplayDevice  {
 public:
    // Bounds of laser hardware interface (laser may actually only be able to draw a region smaller than this)
    static const int MAXDEVICEVALUE;
    static const int MINDEVICEVALUE;
 private:
    int PPS;
    int npoints;
    float targetSegmentLen;   // Target segment length -- if we can achieve this with < npoints, then use that, otherwise limit by npoints
    int blankingSkew;   // Blanks should be moved this many samples later in stream
    int intensityPts;		// Number of points to use to draw intensity image
    int preBlanks;	// Blanks before a move begins
    int postBlanks;  // Blanks after a move ends
    float power;	// Laser power (0-100)
    struct etherdream *d;
    std::vector<etherdream_point> pts;  // Points to send/sent to etherdream buffer
    float spacing;  // Separation in world coordinates of point spacing
    int unit;
    Color labelColor;
    Color maxColor;
    bool showLaser;

    std::vector<etherdream_point> getBlanks(int blanks, etherdream_point pos);
    std::vector<etherdream_point> getBlanks(etherdream_point initial, etherdream_point final);

    // Prune points that are not visible
    void prune();
    // Insert blanking as needed, return number of blanks used
    int blanking();

    float lastAllocationLength;	// Length last allocated to this laser
 public:
    Laser(int unit);
    int open();
    void update();
    const std::vector<etherdream_point> &getPoints() const { return pts; }

    float getSpacing() const { return spacing; }
    void setNPoints(int _npoints) { npoints=_npoints;  }
    void setSkew(int _skew) { blankingSkew=_skew; }
    void setPower(float _power) { power=_power; }
    void setIntensityPts(int _intensityPts) { intensityPts=_intensityPts; }
    void setPreBlanks(int n) { preBlanks=n; }
    void setPostBlanks(int n) { postBlanks=n; }
    void setPPS(int _pps) { PPS=_pps; }
    void setVFOV(float vfov) { transform.setVFOV(vfov); }
    void setHFOV(float hfov) { transform.setHFOV(hfov); }

    int getNPoints() const { return npoints;  }
    int getSkew() const  { return blankingSkew; }
    float getPower() const  { return power; }
    int getIntensityPts() const  { return intensityPts; }
    int getPreBlanks() const  { return preBlanks; }
    int getPostBlanks() const { return postBlanks; }
    int getPPS() const { return PPS; }
    float getVFOV() const { return transform.getVFOV(); }
    float getHFOV() const { return transform.getHFOV(); }

    // Convert drawing into a set of etherdream points
    // Takes into account transformation to make all lines uniform brightness (i.e. separation of points is constant in floor dimensions)
    void render(const Drawing &drawing);
    Color getLabelColor() const { return labelColor; }
    Color getMaxColor() const { return maxColor; }
    void setMaxColor(Color c)  { maxColor=c; }
    int getUnit() const { return unit; }
    void enable(bool enable) { showLaser=enable;
    }
    bool isEnabled() const { return showLaser; }
    void toggleEnable() { showLaser=!showLaser; }
    void dumpPoints() const;
    void showTest();
    void showIntensity();	// Pattern for measuring laser intensity
    void showCalibration(); 	// Show calibration points
    void showOutline(const Bounds &bounds);

    // Determine fraction of the object (in floor coordinates) is in the field of view of the given laser
    float fracOnScreen(const CPoints &cpts) const;
    // Determine fraction of points (in floor coordinates) whose view from origin is obstructed by other objects
    // Only objects that are more than originGap from origin and more than targetGap from the point of interest are considered blockages
    float fracShadowed(const CPoints &cpts, const Ranges &ranges, float originGap, float targetGap) const;
    // Remove shadowed points from a set of CPoints
    CPoints removeShadowed(const CPoints &cpts, const Ranges &ranges, float originGap, float targetGap) const;
    // Compute mean distance of object from given point (typcial from laser origin or center of project)
    float meanDistance(const CPoints &cpts) const;

    // Get length assigned to this laser in last allocation (used for load balancing)
    float getLastAllocationLength() const { return lastAllocationLength; }
    void setLastAllocationLength(float len) { lastAllocationLength=len; }
};
