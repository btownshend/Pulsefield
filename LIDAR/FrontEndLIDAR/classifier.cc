#include <string>
#include <math.h>
#include <mat.h>
#include "classifier.h"
#include "parameters.h"
#include "dbg.h"

Classifier::Classifier(): bg() {
}

std::set<unsigned int> Classifier::getUniqueClasses() const {
    std::set<unsigned int> result;
    for (unsigned int i=0;i<classes.size();i++) 
	if (classes[i]>MAXSPECIAL)
	    result.insert(classes[i]);
    return result;
}

int Classifier::getfirstindex(unsigned int c) const {
    for (unsigned int i=0;i<classes.size();i++) 
	if (classes[i]==c)
	    return i;
    return -1;
}

int Classifier::getlastindex(unsigned int c) const {
    for (int i=(int)classes.size()-1;i>=0;i--) 
	if (classes[i]==c)
	    return i;
    return -1;
}

void Classifier::update(const SickIO &sick) {
    char dbgstr[100];
    sprintf(dbgstr,"Frame.%d",sick.getFrame());

    targets.clear();
    classes.resize(sick.getNumMeasurements());
    shadowed[0].resize(sick.getNumMeasurements());
    shadowed[1].resize(sick.getNumMeasurements());
    for (unsigned int i=0;i<classes.size();i++) {
	shadowed[0][i]=false;
	shadowed[1][i]=false;
    }
    const unsigned int *srange=sick.getRange(0);
    bg.update(sick);
    std::vector<float> isbg = bg.isbg(sick);
    nextclass=MAXSPECIAL+1;
    for (unsigned int i=0;i<classes.size();i++) {
	if (isbg[i]>MINBGFREQ) {
	    classes[i]=BACKGROUND;
	    continue;
	}
	dbg(dbgstr,20) << "S[" << i << "] angle=" << std::fixed << std::setprecision(1) << sick.getAngle(i) << ", range=" << std::setprecision(0) << srange[i] << ", xy=" << sick.getPoint(i) << ", isbg=" << std::setprecision(3) << isbg[i] << " ";
	if (isbg[i]>0 &&i>0 && i<classes.size()-1&&srange[i]<srange[i-1]-MAXLEGDIAM&&isbg[i-1]>MINBGFREQ&&srange[i]<srange[i+1]-MAXLEGDIAM&&isbg[i+1]>MINBGFREQ) {
	    // isolated point, not shadowed on either side
	    classes[i]=NOISE;
	    dbgn(dbgstr,20) << "NOISE" << std::endl;
	    continue;
	}
	// Check if close to a prior target
	bool newclass=true;
	for (int j=i-1;j>=0;j--) {
	    float dist=sick.distance(i,j);
	    if (dist < MAXTGTSEP && classes[j]>MAXSPECIAL) {
		classes[i]=classes[j];
		dbgn(dbgstr,20) << "PRIOR " << classes[i]  << std::endl;
		newclass=false;
		break;
	    }
	    if (dist<= MAXLEGDIAM && classes[j]>MAXSPECIAL && classes[j]!=classes[i-1]) {
		// Could be a leg visible on both sides of a closer leg
		classes[i]=classes[j];
		dbgn(dbgstr,20) << "SPLIT " << classes[i]  << std::endl;
		newclass=false;
		break;
	    }
	    if (srange[j]>srange[i]+MAXCLASSSIZE)
		// A discontinuity which is not shadowed, stop searching
		break;
	}
	if (newclass) {
	    classes[i]=nextclass;
	    dbgn(dbgstr,20) << "NEW " << classes[i]  << std::endl;
	    nextclass++;
	}
    }

    if (nextclass==MAXSPECIAL+1)
	// No targets, done.
	return;

    // Check for solitary classes not too far from adjacent classes
    for (unsigned int i=1;i<classes.size()-1;i++) {
	if (classes[i]>MAXSPECIAL && classes[i-1]!=classes[i] && classes[i+1]!=classes[i]) {
	    // Scans of the first point of a target are sometimes farther off
	    //A single point class just before or after this, with a wider sep theshold, is probably the same target
	    for (unsigned int j=i-1;j<=i+1;j+=2) {
		if (classes[j]>MAXSPECIAL) {
		    float delta=sick.distance(i,j);
		    if (delta < 2*MAXTGTSEP) {
			dbg(dbgstr,20) << "Reassigned singleton " << i << "(class " << classes[i]  << ") with delta=" << delta << " to class " << classes[j] << std::endl;
			classes[i]=classes[j];
			break;
		    }
		} 
	    }
	}
    }

    // Check shadowing
    for (unsigned int i=0;i<classes.size();i++) {
	if (classes[i]>MAXSPECIAL) {
	    if (i>0 && srange[i-1]<srange[i] && classes[i-1]!=classes[i])
		shadowed[0][i] = true;
	    if (i<classes.size()-1 && srange[i+1]<srange[i] && classes[i+1]!=classes[i])
		shadowed[1][i] = true;
	}
    }
    
    // Eliminate small classes
    for (unsigned int c=MAXSPECIAL+1;c<nextclass;c++) {
	int firstindex=getfirstindex(c);
	if (firstindex<0 || shadowed[0][firstindex])
	    continue;
	int lastindex=getlastindex(c);
	assert(lastindex>=0);
	if (shadowed[1][lastindex])
	    continue;
	float dist=sick.distance(firstindex,lastindex);
	float scanwidth=(srange[firstindex]+srange[lastindex])/2.0*sick.getScanRes()*M_PI/180;
	if (dist+scanwidth < MINTARGET) {
	    float bgprob=0;
	    float bgcnt=0;
	    for (int i=firstindex;i<=lastindex;i++)
		if (classes[i]==c) {
		    bgprob+=isbg[i];
		    bgcnt++;
		}
	    bgprob/=bgcnt;
	    dbg(dbgstr,20) << "Target class " << c << " (" << firstindex << ":" << lastindex << ") has size " << dist+scanwidth << "<" << MINTARGET << ", bgprob=" << bgprob;
	    if (bgprob>0) {
		dbgn(dbgstr,20) << ": rejected; is probably noise" << std::endl;
		for (int i=firstindex;i<=lastindex;i++)
		    if (classes[i]==c)
			classes[i]=NOISE;
	    } else
		dbgn(dbgstr,20) << ": kept" << std::endl;
	}
    }

    // Split excessively large classes
    for (unsigned int i=0;i<classes.size();i++)
	if (classes[i]>MAXSPECIAL) {
	    int prevpt=i;
	    float maxdelta=0;
	    int bkpt=i;
	    for (unsigned int j=i+1;j<classes.size();j++) {
		if (classes[j]==classes[i]) {
		    float delta=sick.distance(prevpt,j);
		    if (delta>maxdelta) {
			// Largest jump so far
			maxdelta=delta;
			bkpt=j;
		    }
		    // Keep track of last point of this class (could be discontinuous)
		    prevpt=j;
		    float dist=sick.distance(i,j);
		    if (dist>MAXCLASSSIZE) {
			// Class size exceeded, break at biggest jump so far
			dbg(dbgstr,20) << "Splitting class " << classes[i] << "@ " << i << "-" << j << " at " << bkpt << std::endl;
			for (unsigned int k=bkpt;k<classes.size();k++)
			    if (classes[k]==classes[i])
				classes[k]=nextclass;
			nextclass++;
			i=bkpt;
			break;
		    }
		}
	    }
	}

    // Compact class numbers
    std::vector<bool> present(nextclass,false);
    for (unsigned int i=0;i<classes.size();i++)
	present[classes[i]]=true;
    std::vector<unsigned int>mapped(nextclass);
    unsigned nextup=MAXSPECIAL+1;
    for (unsigned int i=MAXSPECIAL+1;i<nextclass;i++)
	if (present[i]) {
	    mapped[i]=nextup;
	    nextup++;
	}
    for (unsigned int i=0;i<classes.size();i++)
	if (classes[i]>MAXSPECIAL)
	    classes[i]=mapped[classes[i]];
    nextclass=nextup;

    // Build target list
    for (unsigned int c=MAXSPECIAL+1;c<nextclass;c++) {
	std::vector<Point> pts;
	bool first=true;
	bool leftsh, rightsh;
	Point priorpt, nextpt;
	for (unsigned int i=0;i<classes.size();i++) 
	    if (classes[i]==c) {
		pts.push_back(sick.getPoint(i));
		if (first) {
		    leftsh=shadowed[0][i];
		    if (i>0)
			priorpt=sick.getPoint(i-1);
		    first=false;
		}
		rightsh=shadowed[1][i];
		if (i+1<classes.size())
		    nextpt=sick.getPoint(i+1);
		else
		    nextpt=Point();
	    }
	targets.push_back(Target(c,pts,leftsh,rightsh,priorpt,nextpt));
    }

    if (DebugCheck("Classifier",2) ) {
	dbg("Classifier",2) << "Frame " << sick.getFrame() << " ";
	for (unsigned int i=0;i<classes.size();i++)
	    if (classes[i]!=BACKGROUND) {
		unsigned int j;
		for (j=i+1;j<classes.size() && classes[i]==classes[j];j++)
		    ;
		if (j==i+1)
		    dbgn("Classifier",2) << classes[i] << "@" << srange[i] << ": " << i <<  ", ";
		else
		    dbgn("Classifier",2) << classes[i] << "@" << srange[i] << ": " << i <<  "-" << j-1 << ", ";
		i=j;
	    }
	dbgn("Classifier",2) << std::endl;
    }
}

