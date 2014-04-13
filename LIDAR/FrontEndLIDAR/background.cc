#include <assert.h>
#include "background.h"
#include "parameters.h"
#include "dbg.h"

Background::Background() {
    scanRes=0;
    nupdates=0;
}

void Background::setup(const SickIO &sick) {
    if (range[0].size() ==sick.getNumMeasurements())
	return;
    dbg("Background.setup",1) << "Setting up background vectors with " << sick.getNumMeasurements() << " entries." << std::endl;
    for (int i=0;i<NRANGES;i++) {
	range[i].resize(sick.getNumMeasurements());
	freq[i].resize(sick.getNumMeasurements());
    }
    scanRes=sick.getScanRes();
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
std::vector<float> Background::like(const SickIO &sick) const {
    ((Background *)this)->setup(sick);
    std::vector<float> result(sick.getNumMeasurements(),0.0);
    const unsigned int *srange = sick.getRange(0);
    for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
	if (srange[i]>=MAXRANGE || srange[i]<MINRANGE)
	    result[i]=1.0;
	else {
	    // Compute result
	    result[i]=0.0;
	    for (int k=0;k<NRANGES-1;k++) {
		// This is a background pixel if it matches the ranges of this scan's background
		if (freq[k][i]>0 && fabs(srange[i]-range[k][i]) < MINBGSEP )
		    result[i]+=freq[k][i];
	    }
	    if (result[i]<MINBGFREQ) {
		// No strong primary matches, If it matches adjacent scan backgrounds, consider that with a weighting
		for (int k=0;k<NRANGES-1;k++) {
		    if (i>0 && fabs(srange[i]-range[k][i-1])<MINBGSEP) 
			result[i]+=freq[k][i-1]*ADJSCANBGWEIGHT;
		    if (i+1<sick.getNumMeasurements() && fabs(srange[i]-range[k][i+1])<MINBGSEP)
			result[i]+=freq[k][i+1]*ADJSCANBGWEIGHT;
		}
	    }
	    if (result[i]==0 && freq[0][i]>0) {
		// Still no matches, check if is between this and adjacent background
		if (i>0 && freq[0][i-1]>0 && ((srange[i]>range[0][i]) != (srange[i]>range[0][i-1]))) {
		    result[i]=std::min(freq[0][i],freq[0][i-1])*INTERPSCANBGWEIGHT;
		    dbg("Background.like",4) << "Scan " << i << " at " << std::setprecision(0) << std::fixed << srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i-1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
	if (i+1<sick.getNumMeasurements() && freq[0][i+1]>0 && ((srange[i]>range[0][i]) != (srange[i]>range[0][i+1]))) {
		    result[i]=std::min(freq[0][i],freq[0][i+1])*INTERPSCANBGWEIGHT;
		    dbg("Background.like",4) << "Scan " << i << " at " << std::setprecision(0)  <<std::fixed <<  srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i+1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
	    }
	    if (result[i]>1.0) {
		dbg("Background.like",1) << "Scan " << i << " at " << srange[i] << " had prob " << result[i] << "; reducing to 1.0"  << std::endl;
		result[i]=1.0;
	    }
	}
	// TODO: This is not a correct likelihood -- need to have it reflect the p(obs|bg) in the same way that we have p(obs|target) so they can be compared
	// For now, assume this probability occurs over a range of [-MINBGSEP,MINBGSEP], so pdf= prob/(2*MINBGSEP)  (in meters, since all the other PDF's are in meters)
	result[i]=log(result[i]/(2.0*MINBGSEP/UNITSPERM)); 
    }
    return result;
}

void Background::update(const SickIO &sick, const std::vector<int> &assignments, bool all) {
    setup(sick);
    const unsigned int *srange = sick.getRange(0);
    nupdates++;
    if (nupdates<BGINITFRAMES) {
	// Fast adaptation for BGINITFRAMES
	for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
	    freq[0][i]=1;
	    if (srange[i]>range[0][i]-MINBGSEP)
		range[0][i]=(srange[i]+range[0][i])/2;
	    for (int k=1;k<NRANGES;k++)
		freq[k][i]=0;
	}
	return;
    }

    float tc=UPDATETC;
    for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
	if (assignments[i]!=-1 && !all)
	    continue;
	for (int k=0;k<NRANGES;k++) {
	    // Note allow updates even if range>MAXRANGE, otherwise points slightly smaller than MAXRANGE get biased and have low freq
	    if (fabs(srange[i]-range[k][i]) < MINBGSEP) {
		range[k][i]=srange[i]*1.0f/tc + range[k][i]*(1-1.0f/tc);
		freq[k][i]+=(1.0f-freq[k][i])/tc;
		// Swap ordering if needed
		for (int kk=k;kk>0;kk--)
		    if  (freq[kk][i] > freq[kk-1][i])
			swap(i,kk,kk-1);
		    else
			break;
		// Decrement the rest
		for (int kk=k+1;kk<NRANGES;kk++)
		    freq[kk][i]-=(freq[kk][i]/tc);
		break;
	    } else if (k==NRANGES-1 && srange[i]<MAXRANGE && srange[i]>=MINRANGE) {
		// No matches, inside active area; reset last range value 
		range[k][i]=srange[i];
		freq[k][i]=1.0f/tc;
		for (int kk=k;kk>0;kk--)
		    if  (freq[kk][i] > freq[kk-1][i])
			swap(i,kk,kk-1);
		    else
			break;
	    } else {
		freq[k][i]-=(freq[k][i]/tc);
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
	    *data++=range[j][i]/UNITSPERM;
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
