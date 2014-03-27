#include <math.h>
#include "classifier.h"

static const int debug=1;

// Parameters
static const float MAXTGTSEP=100;
static const float INITLEGDIAM=200;
static const float MAXCLASSSIZE=300;

static const unsigned int debugframe=482;

Classifier::Classifier(): bg() {
}

void Classifier::update(const SickIO &sick, unsigned int *result) {
    bool fdebug=false;
    if (sick.getFrame() == debugframe)
	fdebug=true;

    unsigned char isbg[SickIO::MAXMEASUREMENTS];
    const unsigned int *srange=sick.getRange(0);
    bg.update(sick,isbg);
    int nextclass=MAXSPECIAL+1;
    for (int i=0;i<sick.getNumMeasurements();i++) {
	if (isbg[i]) {
	    result[i]=BACKGROUND;
	    continue;
	}
	// Check if close to a prior target
	if (fdebug)
	    printf("%d: ",i);
	bool newclass=true;
	for (int j=i-1;j>=0;j--) {
	    float dist=sqrt(pow(sick.getX(i)-sick.getX(j),2) + pow(sick.getY(i)-sick.getY(j),2));
	    if (fdebug)
		printf("class[%d]=%d, dist=%f ", j, result[j],dist);
	    if (dist < MAXTGTSEP && result[j]>MAXSPECIAL) {
		result[i]=result[j];
		newclass=false;
		break;
	    }
	    if (dist< INITLEGDIAM*1.1 && result[j]>MAXSPECIAL && result[j]!=result[i-1]) {
		// Could be a leg visible on both sides of a closer leg
		result[i]=result[j];
		newclass=false;
		break;
	    }
	    if (srange[j]>srange[i]+MAXCLASSSIZE)
		// A discontinuity which is not shadowed, stop searching
		break;
	}
	if (newclass) {
	    result[i]=nextclass;
	    nextclass++;
	}
	if  (fdebug)
	    printf("class=%d\n", result[i]);
    }

    if (debug && nextclass>MAXSPECIAL+1) {
	printf("Frame %d: ", sick.getFrame());
	for (int i=0;i<sick.getNumMeasurements();i++)
	    if (result[i]!=BACKGROUND) {
		int j;
		for (j=i+1;j<sick.getNumMeasurements() && result[i]==result[j];j++)
		    ;
		if (j==i+1)
		    printf("%d@%d:%d, ",result[i],srange[i],i);
		else
		    printf("%d@%d:%d-%d, ",result[i],srange[i],i,j-1);
		i=j;
	    }
	printf("\n");
    }
}
