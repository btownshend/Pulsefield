/*
 * classifier.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include <vector>
#include <set>
#include "background.h"
#include "target.h"

class Classifier {
    Background bg;
    std::vector<unsigned int> classes;
    std::vector<float> bgprob;
    std::vector<bool> shadowed[2];
    Targets targets;
    unsigned int nextclass;
public:
    enum { BACKGROUND=0, OUTSIDE=1, NOISE=2, MAXSPECIAL=2 };
    Classifier();
    void update(const SickIO &sick);
    const Targets &getTargets() const { return targets; }
    std::set<unsigned int> getUniqueClasses() const;
    Point getClassCenter(const SickIO *sick, int c) const;
    const std::vector<unsigned int> &getclasses() const { return classes; }
    const std::vector<bool> &getshadowed(int i) const { return shadowed[i]; }
    const Background *getBackground() const { return &bg; }
    mxArray *convertToMX() const;
    // Get first,last index of given class; -1 if not found
    int getfirstindex(unsigned int c) const;
    int getlastindex(unsigned int c) const;
    const std::vector<float> getBgProb() const { return bgprob; }
};

#endif  /* CLASSIFIER_H_ */
