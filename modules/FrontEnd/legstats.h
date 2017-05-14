#pragma once

#include <ostream>

class Person;

// Statistics shared between 2 legs
class LegStats {
    float sep,sepSigma; 	// average leg separation in meters
    bool updateSep;	// True to update sep
    float leftness;			// Leftness is >0 when leg0 is the left leg
    float facing,facingSEM;	// Direction in radians they are facing
 public:
    LegStats();
    float getSep() const { return sep; }
    float getSepSigma() const { return sepSigma; }
    float getLeftness() const { return leftness; }
    float getFacing() const { return facing; }
    float getFacingSEM() const { return facingSEM; }

    void update(const Person &p);
    friend std::ostream &operator<<(std::ostream &s, const LegStats &ls);
};
