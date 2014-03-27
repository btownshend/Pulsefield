/*
 * tracker.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef TRACKER_H_
#define TRACKER_H_

#include "classifier.h"
#include "sickio.h"

class Tracker {
    Classifier classifier;
public:
    Tracker();
    // Track people and send update messages
    void track(const SickIO &sick);
    const Classifier *getClassifier() const { return &classifier; }
    mxArray *convertToMX() const;
};

#endif  /* TRACKER_H_ */
