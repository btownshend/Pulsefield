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

class Background {
    static const int NRANGES=3;
    static const int UPDATETC=50*60;

    std::vector<float> range[NRANGES];   // Range in mm of background for NRANGES values/scan
    std::vector<float> freq[NRANGES];
    unsigned int maxrange,minrange;
    float scanRes;

    void swap(int k, int i, int j);
public:
    Background();
    std::vector<bool> isbg(const SickIO &sick) const;
    void update(const SickIO &sick);
    void setmaxrange(float _maxrange) {
	maxrange=_maxrange;
    }
    float getmaxrange() { return maxrange; }
    mxArray *convertToMX() const;
};

#endif  /* BACKGROUND_H_ */
