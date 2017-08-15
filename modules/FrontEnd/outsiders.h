#pragma once

#include "sickio.h"

// Class to keep track of hits outside the pulsefield
class Outsiders {
    static const int innerRadius=5000;   // Inner radius
    static const int outerRadius=6000 ; // Outder
    static const int nDivisions=360;  // Divide circle into this many points
    static const int life=10;   // How many frames to hits survive
    int curFrame;
    int lastHit[nDivisions];
    Point center;
 public:
    Outsiders(Point c) { center=c; }
    void update( Point w);  // Update 
    void update(int frame, const SickIO *s);
    void dump() const;
    void sendMessages(const char *host, int port) const;
};
