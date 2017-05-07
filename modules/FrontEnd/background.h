/*
 * background.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once
#include <vector>
#include <mat.h>
#include "lo/lo.h"

class Vis;
class World;
class SickIO;

class Background {
    static const int NRANGES=5;
    std::vector<float> currentRange;		// Last range received

    // Background constists of multiple ranges, as follows:
    // 0 - most distant background
    // 1..NRANGES-2 - next most frequently seen background
    // NRANGES-1 - potential new background, reset anytime a new scan shows a point that falls outside any of the prior background ranges
    // These can be seen as a N-component gaussian mixture model, with an additional pending range
    std::vector<float> angle;   // Angle of scan for each point
    std::vector<float> range[NRANGES];   // Range in mm of background for NRANGES values/scan (mean of Gaussian)
    std::vector<float> sigma[NRANGES];  // Sigma for Gaussian for this range
    std::vector<float> freq[NRANGES];
    std::vector<int> consecutiveInvisible[NRANGES];

    void swap(int k, int i, int j);
    void setup(const SickIO &sick);
    int bginit;			// Non-zero while initializing background
public:
    Background();
    // Return probability of each scan pixel being part of background (fixed structures not to be considered targets)
    std::vector<float> like(const Vis &vis, const World &world) const;
    void update(const SickIO &sick, const std::vector<int> &assignments, bool all=false);
    mxArray *convertToMX() const;
    const std::vector<float> &getRange(int i) const { return range[i]; }
    const std::vector<float> &getFreq(int i) const { return freq[i]; }
    const std::vector<float> &getSigma(int i) const { return sigma[i]; }
    float getAngleRad(int i) const { return angle[i]; }

    // Send /pf/background OSC message
    void sendMessages(lo_address &addr,int scanpt) const;

    bool isInitializing() const { return bginit>0; }
    int numRanges() const { return NRANGES-1; }  // 1 less than internal since last one is a placeholder for new values
};
