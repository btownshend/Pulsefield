#include <assert.h>
#include "background.h"

Background::Background() {
    maxrange=5000;
    scanRes=0;
}

void Background::swap(int k, int i, int j) {
    // Swap range[i][k] with range[j][k]
    float tmprange=range[i][k];
    range[i][k]=range[j][k];
    range[j][k]=tmprange;
    float tmpfreq=freq[i][k];
    freq[i][k]=freq[j][k];
    freq[j][k]=tmpfreq;
}

std::vector<bool> Background::update(const SickIO &sick) {
    std::vector<bool> result(sick.getNumMeasurements(),false);
    const unsigned int *srange = sick.getRange(0);
    for (int i=0;i<NRANGES;i++) {
	range[i].resize(sick.getNumMeasurements());
	freq[i].resize(sick.getNumMeasurements());
    }
    scanRes=sick.getScanRes();
    for (int i=0;i<sick.getNumMeasurements();i++) {
	if (srange[i]>=maxrange)
	    result[i]=true;
	else {
	    for (int k=0;k<NRANGES;k++)
		freq[k][i]*=(1.0-1.0f/UPDATETC);
	    result[i]=false;
	    for (int k=0;k<NRANGES;k++) {
		if (abs(srange[i]-range[k][i]) < MINBGSEP) {
		    result[i]=k<NRANGES-1;
		    range[k][i]=srange[i]*1.0f/UPDATETC + range[k][i]*(1-1.0f/UPDATETC);
		    freq[k][i]+=1.0f/UPDATETC;
		    // Swap ordering if needed
		    if  (k>0 && freq[k][i] > freq[k-1][i])
			swap(i,k,k-1);
		    break;
		} else if (k==NRANGES-1) {
		    // No matches, reset last range value 
		    range[k][i]=srange[i];
		    freq[k][i]=1.0f/UPDATETC;
		    if (freq[k][i] > freq[k-1][i])
			swap(i,k,k-1);
		}
	    }
	}
    }
    return result;
}

mxArray *Background::convertToMX() const {
    const char *fieldnames[]={"range","angle","freq"};
    mxArray *bg = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pRange = mxCreateDoubleMatrix(NRANGES,range[0].size(),mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (int j=0;j<NRANGES;j++)
	for (unsigned int i=0;i<range[0].size();i++)
	    *data++=range[j][i]/1000.0;
    mxSetField(bg,0,"range",pRange);

    mxArray *pAngle = mxCreateDoubleMatrix(1,range[0].size(),mxREAL);
    assert(pAngle!=NULL);
    data=mxGetPr(pAngle);
    for (unsigned int i=0;i<range[0].size();i++)
	*data++=(i-(range[0].size()-1)/2.0)*scanRes*M_PI/180;
    mxSetField(bg,0,"angle",pAngle);

    mxArray *pFreq = mxCreateDoubleMatrix(NRANGES,range[0].size(),mxREAL);
    assert(pFreq!=NULL);
    data=mxGetPr(pFreq);
    for (int j=0;j<NRANGES;j++)
	for (unsigned int i=0;i<range[0].size();i++)
	    *data++=freq[j][i];
    mxSetField(bg,0,"freq",pFreq);

    if (mxSetClassName(bg,"Background")) {
	fprintf(stderr,"Unable to convert background to a Matlab class\n");
    }
    return bg;
}
