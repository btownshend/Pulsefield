/*
 * likelihood.h
 *
 *  Created on: Mar 30, 2014
 *      Author: bst
 */

#ifndef LIKELIHOOD_H_
#define LIKELIHOOD_H_

class Assignment {
 public:
    int track;
    const Target *target1, *target2;
    float like;
    Assignment(int t, const Target *t1, const Target *t2, float l) { track=t; target1=t1; target2=t2; like=l; }
    void print() const { printf("\tTrack %3d, Classes %2d,%2d, Like %f\n", track, (target1!=NULL)?target1->getClass():1, (target2!=NULL)?target2->getClass():1, like); }
};

class Likelihood {
    std::vector<Assignment> entries;
public:
    void add(const Assignment &a) {
	entries.push_back(a);
    }
    Assignment maxLike() const { 
	assert (entries.size()>0);
	float best=0;
	for (int i=1;i<entries.size();i++)
	    if (entries[i].like>entries[best].like)
		best=i;
	return entries[best];
    }
    void remove(Assignment a) {
	std::vector<Assignment>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ) {
	    if ((a.target1!=NULL && (a.target1==iter->target1 || a.target1==iter->target2)) || (a.target2!=NULL && (a.target2==iter->target2 || a.target2==iter->target1)) || a.track==iter->track)
		iter = entries.erase(iter);
	    else
		++iter;
	}
    }

    int size() const { return entries.size(); }
    void print() const {
	for (int i=0;i<entries.size();i++)
	    entries[i].print();
    }
};

#endif /* LIKELIHOOD_H_ */
