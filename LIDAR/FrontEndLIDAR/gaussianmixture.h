/*
 * gaussianmixture.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once

#include <ostream>
#include <list>
#include <ml/ml.hpp>

// A Gaussian mixture model
class GaussianMixture {
    cv::EM model;
    std::list<float> history;
    int maxHistoryLen;
    int numMixtures;
public:
    // Create a Gaussian mixture model that keeps the last maxHistoryLen samples
 GaussianMixture(int nmixtures,int _maxHistoryLen): model(nmixtures) { maxHistoryLen=_maxHistoryLen; numMixtures=nmixtures; }
    // Update the mixture model with the given x
    void add(float x) { history.push_back(x); while (history.size()>maxHistoryLen) history.pop_front(); };
    void retrain();

    // Get value of likelihood at x
    float like(float x) const;

    std::vector<float> getMeans() const;
    std::vector<float> getCovs() const;
    std::vector<float> getWeights() const;

    int getNumMixtures() const { return numMixtures; }
    friend std::ostream &operator<<(std::ostream &s, const GaussianMixture &g);
};
