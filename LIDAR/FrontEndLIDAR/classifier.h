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
    std::vector<bool> shadowed[2];
    int nextclass;
public:
    Classifier();
    void update(const SickIO &sick);
    const std::vector<unsigned int> getclasses() const { return classes; }
    const std::vector<bool> &getshadowed(int i) const { return shadowed[i]; }
    const Background *getBackground() const { return &bg; }
    mxArray *convertToMX() const;
    // Get first,last index of given class; -1 if not found
    int getfirstindex(unsigned int c) const;
    int getlastindex(unsigned int c) const;
    // Print out 
    void print(const SickIO &sick) const;
};

#endif  /* CLASSIFIER_H_ */
