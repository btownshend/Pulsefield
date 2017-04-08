#pragma once

#include <ostream>

class Person;

// Statistics shared between 2 legs
class LegStats {
    float diam,diamSigma;
    float sep,sepSigma; 	// average leg separation in meters
    bool updateDiam, updateSep;	// True to update diameter, sep
    float leftness;			// Leftness is >0 when leg0 is the left leg
    float facing,facingSEM;	// Direction in radians they are facing
 public:
    LegStats();
    float getDiam() const { return diam; }
    float getDiamSigma() const { return diamSigma; }
    float getSep() const { return sep; }
    float getSepSigma() const { return sepSigma; }
    float getLeftness() const { return leftness; }
    float getFacing() const { return facing; }
    float getFacingSEM() const { return facingSEM; }

    void update(const Person &p);
    void updateDiameter(float newDiam, float newDiamSEM);
    friend std::ostream &operator<<(std::ostream &s, const LegStats &ls);
};
