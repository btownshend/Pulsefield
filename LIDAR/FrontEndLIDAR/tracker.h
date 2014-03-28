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

class Vis;

class Tracker {
public:
    Tracker();
    // Track people and send update messages
    void track(const Vis *vis);
    mxArray *convertToMX() const;
};

#endif  /* TRACKER_H_ */
