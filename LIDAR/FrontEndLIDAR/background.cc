#include <assert.h>
#include "background.h"

Background::Background() {
    maxrange=5000;
    for (int i=0;i<SickIO::MAXMEASUREMENTS;i++)
	for (int k=0;k<NRANGES;k++) {
	    range[i][k]=0.0;
	    freq[i][k]=0.0;
	}
    num_measurements=0;
    scanRes=0;
}

void Background::swap(int k, int i, int j) {
    // Swap range[k][i] with range[k][j]
    float tmprange=range[k][i];
    range[k][i]=range[k][j];
    range[k][j]=tmprange;
    float tmpfreq=freq[k][i];
    freq[k][i]=freq[k][j];
    freq[k][j]=tmpfreq;
}

void Background::update(const SickIO &sick,unsigned char *result) {
    const unsigned int *srange = sick.getRange(0);
    num_measurements=sick.getNumMeasurements();
    scanRes=sick.getScanRes();
    for (int i=0;i<sick.getNumMeasurements();i++) {
	if (srange[i]>=maxrange)
	    result[i]=1;
	else {
	    for (int k=0;k<NRANGES;k++)
		freq[i][k]*=(1.0-1.0f/UPDATETC);
	    result[i]=0;
	    for (int k=0;k<NRANGES;k++) {
		if (abs(srange[i]-range[i][k]) < MINBGSEP) {
		    result[i]=k<NRANGES-1;
		    range[i][k]=srange[i]*1.0f/UPDATETC + range[i][k]*(1-1.0f/UPDATETC);
		    freq[i][k]+=1.0f/UPDATETC;
		    // Swap ordering if needed
		    if  (k>0 && freq[i][k] > freq[i][k-1])
			swap(i,k,k-1);
		    break;
		} else if (k==NRANGES-1) {
		    // No matches, reset last range value 
		    range[i][k]=srange[i];
		    freq[i][k]=1.0f/UPDATETC;
		    if (freq[i][k] > freq[i][k-1])
			swap(i,k,k-1);
		}
	    }
	}
    }
}

mxArray *Background::convertToMX() const {
    const char *fieldnames[]={"range","angle","freq"};
    mxArray *bg = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pRange = mxCreateDoubleMatrix(NRANGES,num_measurements,mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (int j=0;j<NRANGES;j++)
	for (int i=0;i<num_measurements;i++)
	    *data++=range[j][i]/1000.0;
    mxSetField(bg,0,"range",pRange);

    mxArray *pAngle = mxCreateDoubleMatrix(1,num_measurements,mxREAL);
    assert(pAngle!=NULL);
    data=mxGetPr(pAngle);
    for (int i=0;i<num_measurements;i++)
	*data++=(i-(num_measurements-1)/2.0)*scanRes*M_PI/180;
    mxSetField(bg,0,"angle",pAngle);

    mxArray *pFreq = mxCreateDoubleMatrix(NRANGES,num_measurements,mxREAL);
    assert(pFreq!=NULL);
    data=mxGetPr(pFreq);
    for (int j=0;j<NRANGES;j++)
	for (int i=0;i<num_measurements;i++)
	    *data++=freq[j][i];
    mxSetField(bg,0,"freq",pFreq);

    if (mxSetClassName(bg,"Background")) {
	fprintf(stderr,"Unable to convert background to a Matlab class\n");
    }
    return bg;
}
