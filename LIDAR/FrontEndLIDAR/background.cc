#include <assert.h>
#include "background.h"
#include "parameters.h"

Background::Background() {
    scanRes=0;
    nupdates=0;
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

std::vector<bool> Background::isbg(const SickIO &sick) const {
    std::vector<bool> result(sick.getNumMeasurements(),false);
    const unsigned int *srange = sick.getRange(0);
    for (int i=0;i<sick.getNumMeasurements();i++) {
	if (srange[i]>=MAXRANGE || srange[i]<MINRANGE)
	    result[i]=true;
	else {
	    // Compute result
	    for (int k=0;k<NRANGES-1;k++) {
		if (freq[k][i]<MINBGFREQ) {
		    result[i]=false;
		    continue;
		}
		if (k==0 && (abs(srange[i]-range[k][i]) < MINBGSEP ||
			     (i>0 && abs(srange[i]-range[k][i-1])<MINBGSEP) ||
			     (i+1<sick.getNumMeasurements() && abs(srange[i]-range[k][i+1])<MINBGSEP))) {
		    // This is a background pixel if it matches the most common ranges of this or adjecent scans
		    result[i]=true;
		    break;
		} else if (abs(srange[i]-range[k][i])<MINBGSEP) {
		    // or if it matches the 2nd most common AND that range is between the most common background range for the adjacent scan points
		    // This handles the case where a scan is partially occluded by a foreground object and averages between the near and far ranges
		    result[i]=(srange[i]<range[0][std::max(0,i-1)] )!= (srange[i]<range[0][std::min(i+1,sick.getNumMeasurements()-1)]);
		    break;
		}
	    }
	}
    }
    return result;
}

void Background::update(const SickIO &sick) {
    const unsigned int *srange = sick.getRange(0);
    for (int i=0;i<NRANGES;i++) {
	range[i].resize(sick.getNumMeasurements());
	freq[i].resize(sick.getNumMeasurements());
    }
    scanRes=sick.getScanRes();
    nupdates++;
    float tc=std::min(nupdates,UPDATETC);  // Setup so that until we have UPDATETC frames, time constant weights all samples equally
    for (int i=0;i<sick.getNumMeasurements();i++) {
	for (int k=0;k<NRANGES;k++) {
	    freq[k][i]*=(1.0-1.0f/tc);
	    if (srange[i]<MAXRANGE && srange[i]>=MINRANGE) {
		if (abs(srange[i]-range[k][i]) < MINBGSEP) {
		    range[k][i]=srange[i]*1.0f/tc + range[k][i]*(1-1.0f/tc);
		    freq[k][i]+=1.0f/tc;
		    // Swap ordering if needed
		    for (int kk=k;kk>0;kk--)
			if  (freq[kk][i] > freq[kk-1][i])
			    swap(i,kk,kk-1);
			else
			    break;
		    break;
		} else if (k==NRANGES-1) {
		    // No matches, reset last range value 
		    range[k][i]=srange[i];
		    freq[k][i]=1.0f/tc;
		    for (int kk=k;kk>0;kk--)
			if  (freq[kk][i] > freq[kk-1][i])
			    swap(i,kk,kk-1);
			else
			    break;
		}
	    }
	}
    }
}

mxArray *Background::convertToMX() const {
    const char *fieldnames[]={"range","angle","freq"};
    mxArray *bg = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pRange = mxCreateDoubleMatrix(NRANGES,range[0].size(),mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (unsigned int i=0;i<range[0].size();i++)
	for (int j=0;j<NRANGES;j++)
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
    for (unsigned int i=0;i<range[0].size();i++)
	for (int j=0;j<NRANGES;j++)
	    *data++=freq[j][i];
    mxSetField(bg,0,"freq",pFreq);

    if (mxSetClassName(bg,"Background")) {
	fprintf(stderr,"Unable to convert background to a Matlab class\n");
    }
    return bg;
}
