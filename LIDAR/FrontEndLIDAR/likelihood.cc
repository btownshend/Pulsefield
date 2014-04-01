#include <set>
#include <ostream>

#include "likelihood.h"
#include "classifier.h"
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


Likelihood Likelihood::smartassign() {
    Likelihood result;

    std::set<const Target *> alltgts;
    for (unsigned int i=0;i<entries.size();i++) {
	if (entries[i].target1!=NULL) alltgts.insert(entries[i].target1);
	if (entries[i].target2!=NULL) alltgts.insert(entries[i].target2);
    }

    while (size() > 0) {
	unsigned int mincnt=10000;
	const Target *mintarget = NULL;
	int mintrack;

	for (std::set<const Target *>::iterator t=alltgts.begin();t!=alltgts.end();t++) {
	    std::set<int> tracks;
	    for (unsigned int i=0;i<entries.size();i++) {
		if (entries[i].track>=0 && (entries[i].target1==*t ||  entries[i].target2==*t))
		    tracks.insert(entries[i].track);
	    }
	    dbg("Likelihood",3) << "Target: " <<  (*t)->getClass() << ", nentries=" << tracks.size() << std::endl;
	    if (tracks.size() < mincnt && tracks.size()>=1) {
		mincnt=tracks.size();
		mintarget=*t;
		mintrack=*tracks.begin();
	    }
	}
	if (mintarget==NULL)
	    break;
	dbg("Likelihood",2) << "Class " << mintarget->getClass() << " can be mapped to " << mincnt << " people (including " << mintrack << ")" << std::endl;
	if (mincnt==1) {
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
    dbg("Likelihood",2) << "After updates: " << std::endl << *this;
    return greedy();
}


