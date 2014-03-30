#include <math.h>
#include <mat.h>
#include "classifier.h"
#include "parameters.h"

static const int debug=1;

static const unsigned int debugframe=394;

Classifier::Classifier(): bg() {
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
    bool fdebug=false;
    if (sick.getFrame() == debugframe)
	fdebug=true;
    classes.resize(sick.getNumMeasurements());
    shadowed[0].resize(sick.getNumMeasurements());
    shadowed[1].resize(sick.getNumMeasurements());
    for (unsigned int i=0;i<classes.size();i++) {
	shadowed[0][i]=false;
	shadowed[1][i]=false;
    }
    const unsigned int *srange=sick.getRange(0);
    bg.update(sick);
    std::vector<bool> isbg = bg.isbg(sick);
    nextclass=MAXSPECIAL+1;
    for (unsigned int i=0;i<classes.size();i++) {
	if (isbg[i]) {
	    classes[i]=BACKGROUND;
	    continue;
	}
	if (i>0 && i<classes.size()-1&&srange[i]<srange[i-1]&&isbg[i-1]&&srange[i]<srange[i+1]&&isbg[i+1]) {
	    // isolated point, not shadowed on either side
	    if (fdebug)
		printf("NOISE %d angle=%f, range=%d, xy=(%f,%f)\n",i,sick.getAngle(i), srange[i], sick.getX(i), sick.getY(i));
	    classes[i]=NOISE;
	    continue;
	}
	// Check if close to a prior target
	if (fdebug)
	    printf("%d angle=%f, range=%d, xy=(%f,%f): ",i,sick.getAngle(i), srange[i], sick.getX(i), sick.getY(i));
	bool newclass=true;
	for (int j=i-1;j>=0;j--) {
	    float dist=sick.distance(i,j);
	    if (fdebug)
		printf("class[%d]=%d, xy=(%f,%f), dist=%f ", j, classes[j],sick.getX(j),sick.getY(j),dist);
	    if (dist < MAXTGTSEP && classes[j]>MAXSPECIAL) {
		classes[i]=classes[j];
		newclass=false;
		break;
	    }
	    if (dist<= MAXLEGDIAM && classes[j]>MAXSPECIAL && classes[j]!=classes[i-1]) {
		// Could be a leg visible on both sides of a closer leg
		classes[i]=classes[j];
		newclass=false;
		break;
	    }
	    if (srange[j]>srange[i]+MAXCLASSSIZE)
		// A discontinuity which is not shadowed, stop searching
		break;
	}
	if (newclass) {
	    classes[i]=nextclass;
	    nextclass++;
	}
	if  (fdebug)
	    printf("class=%d\n", classes[i]);
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
	    printf("Target class %d (%d:%d) has size %.2f<%.2f, is probably noise\n", c, firstindex,lastindex, dist+scanwidth,MINTARGET);
	    for (int i=firstindex;i<=lastindex;i++)
		if (classes[i]==c)
		    classes[i]=NOISE;
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
			printf("Splitting class %d@ %d-%d at %d\n", classes[i],i,j,bkpt);
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

    if (debug) {
	print(sick);
    }
}

void Classifier::print(const SickIO &sick) const {
    const unsigned int *srange=sick.getRange(0);
    printf("Frame %d: ", sick.getFrame());
    for (unsigned int i=0;i<classes.size();i++)
	if (classes[i]!=BACKGROUND) {
	    unsigned int j;
	    for (j=i+1;j<classes.size() && classes[i]==classes[j];j++)
		;
	    if (j==i+1)
		printf("%d@%d:%d, ",classes[i],srange[i],i);
	    else
		printf("%d@%d:%d-%d, ",classes[i],srange[i],i,j-1);
	    i=j;
	}
    printf("\n");
}

