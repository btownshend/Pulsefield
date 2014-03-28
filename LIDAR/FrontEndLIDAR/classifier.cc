#include <math.h>
#include <mat.h>
#include "classifier.h"

static const int debug=1;

// Parameters
static const float MAXTGTSEP=100;
static const float INITLEGDIAM=200;
static const float MAXCLASSSIZE=300;

static const unsigned int debugframe=482;

Classifier::Classifier(): bg() {
}

void Classifier::update(const SickIO &sick) {
    bool fdebug=false;
    if (sick.getFrame() == debugframe)
	fdebug=true;
    classes.resize(sick.getNumMeasurements());

    const unsigned int *srange=sick.getRange(0);
    std::vector<bool> isbg = bg.update(sick);
    int nextclass=MAXSPECIAL+1;
    for (unsigned int i=0;i<classes.size();i++) {
	if (isbg[i]) {
	    classes[i]=BACKGROUND;
	    continue;
	}
	// Check if close to a prior target
	if (fdebug)
	    printf("%d: ",i);
	bool newclass=true;
	for (int j=i-1;j>=0;j--) {
	    float dist=sqrt(pow(sick.getX(i)-sick.getX(j),2) + pow(sick.getY(i)-sick.getY(j),2));
	    if (fdebug)
		printf("class[%d]=%d, dist=%f ", j, classes[j],dist);
	    if (dist < MAXTGTSEP && classes[j]>MAXSPECIAL) {
		classes[i]=classes[j];
		newclass=false;
		break;
	    }
	    if (dist< INITLEGDIAM*1.1 && classes[j]>MAXSPECIAL && classes[j]!=classes[i-1]) {
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

    if (debug && nextclass>MAXSPECIAL+1) {
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
}
