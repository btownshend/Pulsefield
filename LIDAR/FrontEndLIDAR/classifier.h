/*
 * classifier.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include "background.h"

class Classifier {
    enum classes { BACKGROUND=0, OUTSIDE=1, NOISE=2, MAXSPECIAL=2 };
    Background bg;
public:
    Classifier();
    void update(const SickIO &sick, unsigned int *result);
    const Background *getBackground() const { return &bg; }
};

#endif  /* CLASSIFIER_H_ */
