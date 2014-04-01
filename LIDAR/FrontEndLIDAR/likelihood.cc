#include <ostream>
#include "likelihood.h"
#include "target.h"
#include "dbg.h"

std::ostream& operator<<(std::ostream &s, const Assignment &a) {
    s << "\tTrack " << a.track << ", Classes " << ( (a.target1!=NULL)?a.target1->getClass():1 ) << "," << ((a.target2!=NULL)?a.target2->getClass():1) << ", Like:" << a.like << std::endl;
    return s;
}

std::ostream& operator<<(std::ostream &s, const Likelihood &l) {
    for (unsigned int i=0;i<l.entries.size();i++)
	s << l.entries[i];
    return s;
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


