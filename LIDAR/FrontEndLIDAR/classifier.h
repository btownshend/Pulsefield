/*
 * classifier.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include <vector>
#include "background.h"

class Classifier {
    enum classes { BACKGROUND=0, OUTSIDE=1, NOISE=2, MAXSPECIAL=2 };
    Background bg;
    std::vector<unsigned int> classes;
    int num_measurements;
public:
    Classifier();
    void update(const SickIO &sick);
    const std::vector<unsigned int> getclasses() const { return classes; }
    const Background *getBackground() const { return &bg; }
    mxArray *convertToMX() const;
};

#endif  /* CLASSIFIER_H_ */
