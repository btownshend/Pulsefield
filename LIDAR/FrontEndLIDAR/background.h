/*
 * background.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_
#include "sickio.h"

class Background {
    static const int MINBGSEP=100;	// mm
    static const int NRANGES=3;
    static const int UPDATETC=50*60;

    float range[SickIO::MAXMEASUREMENTS][NRANGES];			// Range in mm of background for NRANGES values/scan
    float freq[SickIO::MAXMEASUREMENTS][NRANGES];
    float maxrange;
    float scanRes;

    int num_measurements;
    void swap(int k, int i, int j);
public:
    Background();
    void update(const SickIO &sick,unsigned char *result);
    void setmaxrange(float _maxrange) {
	maxrange=_maxrange;
    }
    float getmaxrange() { return maxrange; }
    mxArray *convertToMX() const;
};

#endif  /* BACKGROUND_H_ */
