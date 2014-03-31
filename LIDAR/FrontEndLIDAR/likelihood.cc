#include "likelihood.h"
#include "target.h"


void Assignment::print() const {
    printf("\tTrack %3d, Classes %2d,%2d, Like %f\n", track, (target1!=NULL)?target1->getClass():1, (target2!=NULL)?target2->getClass():1, like);
}

Likelihood Likelihood::greedy() {
    Likelihood result;

    // Greedy assignment
    while (size() > 0) {
	Assignment a=maxLike();
	result.add(a);
	remove(a);
    }
    return result;
}


