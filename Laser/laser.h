#pragma once

#include <vector>
#include "etherdream_bst.h"
#include "displaydevice.h"
#include "color.h"

class Drawing;

class Laser: public DisplayDevice  {
    int PPS;
    int npoints;
    int blankingSkew;   // Blanks should be moved this many samples later in stream
    int preBlanks;	// Blanks before a move begins
    int postBlanks;  // Blanks after a move ends
    struct etherdream *d;
    std::vector<etherdream_point> pts;
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
 public:
    Laser(int unit);
    int open();
    void update();
    const std::vector<etherdream_point> &getPoints() const { return pts; }
    float getSpacing() const { return spacing; }
    void setPoints(int _npoints) { npoints=_npoints; }
    void setSkew(int _skew) { blankingSkew=_skew; }
    void setBlanks(int _pre, int _post) { preBlanks=_pre; postBlanks=_post; }
    void setPPS(int _pps) { PPS=_pps; }
    // Convert drawing into a set of etherdream points
    // Takes into account transformation to make all lines uniform brightness (i.e. separation of points is constant in floor dimensions)
    void render(const Drawing &drawing);
    Color getLabelColor() const { return labelColor; }
    Color getMaxColor() const { return maxColor; }
    int getUnit() const { return unit; }
    void toggleLaser() { showLaser=!showLaser; }
    void dumpPoints() const;
};
