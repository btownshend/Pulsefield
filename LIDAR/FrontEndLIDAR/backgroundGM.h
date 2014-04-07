/*
 * background.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_
#include <vector>
#include "sickio.h"
#include "gaussianmixture.h"

class Background {
    std::vector<GaussianMixture> mixtures;
    unsigned int nextToUpdate;	// Next scan line to update (to distribute update time amongst scans)
    void sizeCheck(int n);
    float scanRes;
public:
    Background();
    // Return likelihood of each scan pixel being part of background (fixed structures not to be considered targets)
    std::vector<float> like(const SickIO &sick) const;
    void update(const SickIO &sick, const std::vector<int> &assignments);
    mxArray *convertToMX() const;
};

#endif  /* BACKGROUND_H_ */
