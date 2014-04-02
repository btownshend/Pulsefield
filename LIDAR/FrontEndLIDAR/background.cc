#include <assert.h>
#include "background.h"
#include "parameters.h"
#include "dbg.h"

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

// Return probability of each scan pixel being part of background (fixed structures not to be considered targets)
std::vector<float> Background::isbg(const SickIO &sick) const {
    std::vector<float> result(sick.getNumMeasurements(),false);
    const unsigned int *srange = sick.getRange(0);
    for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
	if (srange[i]>=MAXRANGE || srange[i]<MINRANGE)
	    result[i]=1.0;
	else {
	    // Compute result
	    result[i]=0.0;
	    for (int k=0;k<NRANGES-1;k++) {
		// This is a background pixel if it matches the ranges of this scan's background
		if (freq[k][i]>0 && abs(srange[i]-range[k][i]) < MINBGSEP )
		    result[i]+=freq[k][i];
	    }
	    if (result[i]<MINBGFREQ) {
		// No strong primary matches, If it matches adjacent scan backgrounds, consider that with a weighting
		for (int k=0;k<NRANGES-1;k++) {
		    if (i>0 && abs(srange[i]-range[k][i-1])<MINBGSEP) 
			result[i]+=freq[k][i-1]*ADJSCANBGWEIGHT;
		    if (i+1<sick.getNumMeasurements() && abs(srange[i]-range[k][i+1])<MINBGSEP)
			result[i]+=freq[k][i+1]*ADJSCANBGWEIGHT;
		}
	    }
	    if (result[i]==0 && freq[0][i]>0) {
		// Still no matches, check if is between this and adjacent background
		if (i>0 && freq[0][i-1]>0 && (srange[i]>range[0][i] != srange[i]>range[0][i-1])) {
		    result[i]=std::min(freq[0][i],freq[0][i-1])*INTERPSCANBGWEIGHT;
		    dbg("Background.isbg",4) << "Scan " << i << " at " << std::setprecision(0) << std::fixed << srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i-1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
		if (i+1<sick.getNumMeasurements() && freq[0][i+1]>0 && (srange[i]>range[0][i] != srange[i]>range[0][i+1])) {
		    result[i]=std::min(freq[0][i],freq[0][i+1])*INTERPSCANBGWEIGHT;
		    dbg("Background.isbg",4) << "Scan " << i << " at " << std::setprecision(0)  <<std::fixed <<  srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i+1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
	    }
	    if (result[i]>1.0) {
		dbg("Background.isbg",1) << "Scan " << i << " at " << srange[i] << " had prob " << result[i] << "; reducing to 1.0"  << std::endl;
		result[i]=1.0;
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
    for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
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
