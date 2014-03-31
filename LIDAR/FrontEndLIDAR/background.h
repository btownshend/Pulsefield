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

    int nupdates;
    std::vector<float> range[NRANGES];   // Range in mm of background for NRANGES values/scan
    std::vector<float> freq[NRANGES];
    float scanRes;

    void swap(int k, int i, int j);
public:
    Background();
    std::vector<bool> isbg(const SickIO &sick) const;
    void update(const SickIO &sick);
    mxArray *convertToMX() const;
};

#endif  /* BACKGROUND_H_ */