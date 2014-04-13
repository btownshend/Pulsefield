/*
 * lookuptable.h
 *
 *  Created on: April 4, 2014
 *      Author: bst
 */

#ifndef LOOKUPTABLE_H_
#define LOOKUPTABLE_H_
#include <vector>
#include <numeric>

// Uniformly spaced linear-interpolating lookup table
class LookupTable {
    float minx,step;
    std::vector<float> values;
 public:
    LookupTable() {
	minx=0;
	step=0;
    }
    LookupTable(float _minx, float _maxx, int nstep) {
	minx=_minx;
	step=(_maxx-_minx)/nstep;
	values.resize(nstep);
    }
    float &operator[](int i) { return values[i]; }
    void push_back(float y) {
	values.push_back(y);
    }
    float lookup(float x) const {
	float findex=(x-minx)/step;
	if (findex<0)
	    return values.front();
	if (findex>=values.size()-1)
	    return values.back();
	int index=(int)findex;
	int frac=findex-index;
	return values[index]*(1-frac)+values[index+1]*frac;
    }
    unsigned int size() const  { return values.size(); }
    float sum() const { return std::accumulate(values.begin(),values.end(),0.0); }
};

// Get a lookup table for the distribution of leg separation probabilities assuming position is zero-mean gaussian and separation is log-normal with given statistics
LookupTable getLegSepLike(float sepmu,float sepsigma,float possigma);
#endif  /* LOOKUPTABLE_H_ */
