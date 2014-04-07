#include <assert.h>
#include "background.h"
#include "parameters.h"
#include "dbg.h"

Background::Background() {
    nextToUpdate=0;
}

// Return probability of each scan pixel being part of background (fixed structures not to be considered targets)
std::vector<float> Background::like(const SickIO &sick) const {
    ((Background *)this)->sizeCheck(sick.getNumMeasurements());

    std::vector<float> result(sick.getNumMeasurements());
    const unsigned int *srange = sick.getRange(0);
    for (unsigned int i=0;i<sick.getNumMeasurements();i++)
	result[i]=mixtures[i].like(srange[i]);
    return result;
}

void Background::sizeCheck(int n) {
    if (mixtures.size()==0) {
	mixtures.resize(n,GaussianMixture(BGNCLUSTERS,BGHISTORYLEN));
	dbg("Background.update",1) << "Initializing background Gaussian mixture model with " << BGNCLUSTERS << " clusters, history of " << BGHISTORYLEN << " samples and " << mixtures.size() << " scan points." << std::endl;
    }
    assert(n==mixtures.size());
}

void Background::update(const SickIO &sick, const std::vector<int> &assignments) {
    sizeCheck(sick.getNumMeasurements());
    assert(assignments.size()==mixtures.size());
    scanRes=sick.getScanRes();
    const unsigned int *srange = sick.getRange(0);
    for (int i=0;i<mixtures.size();i++) {
	if (assignments[i]==-1)
	    mixtures[i].add(srange[i]);
    }
    // Cycle through doing the EM updates for 1 background pixel each frame
    dbg("Background.update",3) << "Retraining background for scan point " << nextToUpdate << std::endl;
    mixtures[nextToUpdate].retrain();
    nextToUpdate=(nextToUpdate+1)%mixtures.size();
}

mxArray *Background::convertToMX() const {
    const char *fieldnames[]={"range","angle","freq"};
    mxArray *bg = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pRange = mxCreateDoubleMatrix(mixtures[0].getNumMixtures(),mixtures.size(),mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (int j=0;j<mixtures.size();j++) {
	std::vector<float> means = mixtures[j].getMeans();
	for (unsigned int i=0;i<mixtures[0].getNumMixtures();i++)
	    data[i*mixtures.size()+j]=means[i]/1000.0;
    }
    mxSetField(bg,0,"range",pRange);

    mxArray *pAngle = mxCreateDoubleMatrix(1,mixtures.size(),mxREAL);
    assert(pAngle!=NULL);
    data=mxGetPr(pAngle);
    for (unsigned int i=0;i<mixtures.size();i++)
	*data++=(i-(mixtures.size()-1)/2.0)*scanRes*M_PI/180;
    mxSetField(bg,0,"angle",pAngle);

    mxArray *pFreq = mxCreateDoubleMatrix(mixtures[0].getNumMixtures(),mixtures.size(),mxREAL);
    assert(pFreq!=NULL);
    data=mxGetPr(pFreq);
    for (int j=0;j<mixtures.size();j++) {
	std::vector<float> weights = mixtures[j].getWeights();
	for (unsigned int i=0;i<mixtures[0].getNumMixtures();i++)
	    data[i*mixtures.size()+j]=weights[i]/1000.0;
    }
    mxSetField(bg,0,"freq",pFreq);

    if (mxSetClassName(bg,"Background")) {
	fprintf(stderr,"Unable to convert background to a Matlab class\n");
    }
    return bg;
}
