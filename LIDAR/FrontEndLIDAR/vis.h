/*
 * vis.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef VIS_H_
#define VIS_H_
#include "sickio.h"
#include "classifier.h"

class Vis {
    const SickIO *sick;
    Classifier classifier;
public:
    Vis();
    void update(const SickIO *sick);

    // Convert to an mxArray
    mxArray *convertToMX() const;

    const Classifier *getClassifier() const { return &classifier; }
    const SickIO *getSick() const { return sick; }
};

#endif  /* VIS_H_ */
