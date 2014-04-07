#include <set>
#include <ostream>
#include <iomanip>

#include "likelihood.h"
#include "target.h"
#include "parameters.h"
#include "dbg.h"

std::ostream& operator<<(std::ostream &s, const Assignment &a) {
    s << "Track: " << a.track << ", Classes: " << ( (a.target1!=NULL)?a.target1->getClass():1 ) << "," << ((a.target2!=NULL)?a.target2->getClass():1) << ", Like:" << std::setprecision(4) << a.like;
    return s;
}

std::ostream& operator<<(std::ostream &s, const Likelihood &l) {
    for (unsigned int i=0;i<l.entries.size();i++)
	s << "                          " << l.entries[i] << std::endl;
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


Likelihood Likelihood::smartassign() {
    Likelihood result;

    while (size()>0) {
	std::set<const Target *> alltgts;
	for (unsigned int i=0;i<entries.size();i++) {
	    if (entries[i].target1!=NULL) alltgts.insert(entries[i].target1);
	    if (entries[i].target2!=NULL) alltgts.insert(entries[i].target2);
	}

	// If a class can only be assigned to a single ID, then remove any entries that do not assign that class to that ID
	while (size() > 0) {
	    unsigned int mincnt=10000;
	    const Target *mintarget = NULL;
	    float minlike;
	    int mintrack;

	    for (std::set<const Target *>::iterator t=alltgts.begin();t!=alltgts.end();t++) {
		std::set<int> tracks;
		float tlike=-1e99;
		for (unsigned int i=0;i<entries.size();i++) {
		    if (entries[i].track>=0 && (entries[i].target1==*t ||  entries[i].target2==*t)) {
			tracks.insert(entries[i].track);
			if (entries[i].like > tlike)
			    tlike=entries[i].like;
		    }
		}
		dbg("Likelihood",3) << "Target: " <<  (*t)->getClass() << ", nentries=" << tracks.size() << std::endl;
		if ((tracks.size() < mincnt ||  (tracks.size()==mincnt && tlike>minlike)) && tracks.size()>=1) {
		    mincnt=tracks.size();
		    mintarget=*t;
		    mintrack=*tracks.begin();
		    minlike=tlike;
		}
	    }
	    if (mintarget==NULL)
		break;
	    dbg("Likelihood",3) << "Class " << mintarget->getClass() << " can be mapped to " << mincnt << " people (including ID " << mintrack << ") with like <= " << minlike << std::endl;
	    if (mincnt==1 && minlike>MINFORCELIKE) {
		// Can only be assigned to one track, remove all entries that don't assign it to this person
		std::vector<Assignment>::iterator iter;
		for (iter = entries.begin(); iter != entries.end(); ) {
		    if (iter->target1!=mintarget && iter->target2!=mintarget && iter->track==mintrack)
			iter = entries.erase(iter);
		    else
			++iter;
		}
		alltgts.erase(mintarget);
	    } else
		break;
	}
	if (size() > 0)
	    dbg("Likelihood",3) << "After updates: " << std::endl << *this;
	// Now do the maximum likelihood assignment
	Assignment a=maxLike();
	dbg("Likelihood",2) << "Assigning " << a << std::endl;
	result.add(a);
	remove(a);
	// And redo the whole process
    }
    return result;
}


