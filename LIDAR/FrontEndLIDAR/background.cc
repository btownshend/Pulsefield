#include <assert.h>
#include "background.h"
#include "parameters.h"
#include "dbg.h"
#include "normal.h"

Background::Background() {
    scanRes=0;
    bginit=true;
}

void Background::setup(const SickIO &sick) {
    if (range[0].size() ==sick.getNumMeasurements())
	return;
    dbg("Background.setup",1) << "Setting up background vectors with " << sick.getNumMeasurements() << " entries." << std::endl;
    for (int i=0;i<NRANGES;i++) {
	range[i].resize(sick.getNumMeasurements());
	freq[i].resize(sick.getNumMeasurements());
	sigma[i].resize(sick.getNumMeasurements());
	for (int j=0;j<sick.getNumMeasurements();j++)
	    sigma[i][j]=MEANBGSIGMA;
    }
    farnotseen.resize(sick.getNumMeasurements());
    scanRes=sick.getScanRes();
    bginit=true;
}

void Background::swap(int k, int i, int j) {
    // Swap range[i][k] with range[j][k]
    float tmprange=range[i][k];
    range[i][k]=range[j][k];
    range[j][k]=tmprange;
    float tmpfreq=freq[i][k];
    freq[i][k]=freq[j][k];
    freq[j][k]=tmpfreq;
    float tmpsigma=sigma[i][k];
    sigma[i][k]=sigma[j][k];
    sigma[j][k]=tmpsigma;
}

// Return likelihood of each scan pixel being part of background (fixed structures not to be considered targets)
// Note that these are not probabilities -- they can only be compared with equivalently computed likelihoods of the scan pixel being part of a hit
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
		// This is a background pixel if it matches the ranges of this scan's background, or is farther than maximum background
		if (srange[i]>range[k][i] && k==0)
		    result[i]=1.0;
		else 
		    result[i]+=(1-result[i])*freq[k][i]*normpdf(srange[i],range[k][i],sigma[k][i]); 

		// Use adjacent scans
		if (i>0)
		    result[i]+=(1-result[i])*freq[k][i-1]*ADJSCANBGWEIGHT*normpdf(srange[i],range[k][i-1],sigma[k][i-1]);
		if (i+1<sick.getNumMeasurements())
		    result[i]+=(1-result[i])*freq[k][i+1]*ADJSCANBGWEIGHT*normpdf(srange[i],range[k][i+1],sigma[k][i+1]);

		// Check if it is between this and adjacent background
		if (i>0 && ((srange[i]>range[0][i]) != (srange[i]>range[0][i-1]))) {
		    result[i]+=(1-result[i])*freq[0][i]*freq[0][i-1]*INTERPSCANBGWEIGHT/fabs(range[0][i-1]-range[0][i]);
		    //		    dbg("Background.like",4) << "Scan " << i << " at " << std::setprecision(0) << std::fixed << srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i-1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
		if (i+1<sick.getNumMeasurements() && ((srange[i]>range[0][i]) != (srange[i]>range[0][i+1]))) {
		    result[i]=freq[0][i]*freq[0][i+1]*INTERPSCANBGWEIGHT/fabs(range[0][i]-range[0][i+1]);
		    //		    dbg("Background.like",4) << "Scan " << i << " at " << std::setprecision(0)  <<std::fixed <<  srange[i] << " is between adjacent background ranges of " << range[0][i] << " and " << range[0][i+1] << ": result=" << std::setprecision(3) << result[i] << std::endl;
		}
	    }
	    if (result[i]>1.0) {
		dbg("Background.like",1) << "Scan " << i << " at " << srange[i] << " had prob " << result[i] << "; reducing to 1.0"  << std::endl;
		result[i]=1.0;
	    }
	}
	// Make this into a  likelihood -- need to have it reflect the p(obs|bg) in the same way that we have p(obs|target) so they can be compared
	// Problem is that even for very low probs (e.g. .002), likelihood is >>entrylikelihood
	result[i]=log(result[i]*UNITSPERM); 
    }
    return result;
}

void Background::update(const SickIO &sick, const std::vector<int> &assignments, bool all) {
    setup(sick);
    if (bginit && sick.getFrame()>BGINITFRAMES)
	bginit=false;

    const unsigned int *srange = sick.getRange(0);
   
    // range[0] is for the farthest seen in last BGLONGDISTLIFE frames
    // range[1] is the most frequent seen, other than range[0], with exponential decay using UPDATETC
    // range[2] is the last value seen, not matching 0 or 1;   promoted to range[1] if its frequency passes range[1]
    float tc=UPDATETC;
    for (unsigned int i=0;i<sick.getNumMeasurements();i++) {
	if (!all && assignments[i]!=-1)
	    continue;
	// Find which background fits best
	int bestk=-1;
	float bestprob=-1;
	for (int k=0;k<NRANGES;k++) {
	    if (fabs(srange[i]-range[k][i])<3*sigma[k][i]) {
		float p=normpdf(srange[i],range[k][i],sigma[k][i]);
		if (p>bestprob) {
		    bestk=k;
		    bestprob=p;
		}
	    }
	}
	dbg("Background.update",5) << "Best fit for background " << i << " is index " << bestk << " with prob " << std::setprecision(8) << bestprob << std::setprecision(3) << std::endl;
	if (bestk >= 0) {
	    // Normal update
	    float oldrange=range[bestk][i];
	    range[bestk][i]=srange[i]*1.0f/tc + range[bestk][i]*(1-1.0f/tc);
	    freq[bestk][i]+=1.0f/tc;
	    if (!bginit)
		sigma[bestk][i]=std::min((double)MAXBGSIGMA,sqrt(sigma[bestk][i]*sigma[bestk][i]*(1-1.0f/tc)+(srange[i]-range[bestk][i])*(srange[i]-oldrange)*1.0f/tc));
	    if (bestk==0)
		farnotseen[i]=0;
	} else if (srange[i]>range[0][i]) {
	    // New long distance point, update range[0] faster
	    dbg("Background.update",2) << "Farthest background at scan " << i << " moved from " << range[0][i] << " to " << srange[i] << std::endl;
	    float oldrange=range[0][i];
	    range[0][i]=srange[i]*1.0f/FARUPDATETC + range[0][i]*(1-1.0f/FARUPDATETC);
	    if (!bginit)
		sigma[0][i]=std::min((double)MAXBGSIGMA,sqrt(sigma[0][i]*sigma[0][i]*(1-1.0f/FARUPDATETC)+(srange[i]-range[0][i])*(srange[i]-oldrange)*1.0f/FARUPDATETC));
	    freq[0][i]+=1.0f/tc;
	    farnotseen[i]=0;
	} else {
	    // No matches, inside active area; reset last range value 
	    range[NRANGES-1][i]=srange[i];
	    sigma[NRANGES-1][i]=MEANBGSIGMA;
	    freq[NRANGES-1][i]=1.0f/tc;
	}
	// Swap ordering if needed
	if (range[0][i] < range[1][i])
	    // range[0] is always farthest
	    swap(i,0,1);

	bool dodump=false;
	for (int k=NRANGES-1;k>1;k--)
	    if  (freq[k][i] > freq[k-1][i]) {
		// freq[1] should always be > freq[2]
		dbg("Background.update",2) << "Promoting background at scan " << i << " with range=" << range[k][i] << ", freq=" << freq[k][i] << " to level " << k-1 << "; range[0]=" << range[0][i]  << std::endl;
		swap(i,k,k-1);
		dodump=true;
	    } 

	// Rescale so freq adds to 1.0
	float ftotal=0;
	for (int k=0;k<NRANGES;k++)
	    ftotal+=freq[k][i];
	for (int k=0;k<NRANGES;k++)
	    freq[k][i]/=ftotal;

	if (farnotseen[i] > BGLONGDISTLIFE) {
	    dbg("Background.update",2) << "Farthest background at scan " << i << " not seen for " << farnotseen[i] << " frames.  Resetting to " << range[1][i] << std::endl;
	    swap(i,0,1);
	    swap(i,1,2);
	    farnotseen[i]=0;
	    dodump=true;
	}
	//See if we should split any backgrounds
	for (int k=0;k<NRANGES-1;k++) {
	    if (sigma[k][i]==MAXBGSIGMA && freq[k][i]>10*freq[NRANGES-2][i]) {
		dbg("Background.update",2) << "Scan " << i << ", splitting " << k << " (range=" << range[k][i] << ", sigma=" << sigma[k][i] << ", freq=" << freq[k][i] << "), replacing last bg with freq=" << freq[NRANGES-2][i] << std::endl;
		for (int kk=NRANGES-2;kk>k+1;kk--)
		    swap(i,kk,kk-1);
		range[k+1][i]=range[k][i]-0.8*sigma[k][i];
		sigma[k+1][i]=sigma[k][i]*0.6;
		freq[k+1][i]=freq[k][i]/2;
		range[k][i]=range[k][i]+0.8*sigma[k][i];
		sigma[k][i]=sigma[k][i]*0.6;
		freq[k][i]=freq[k][i]/2;
		dodump=true;
	    }
	}
	if (false) {
	    // Disable this for now;  seems that one should dominate and eventually take over with the other one slowly fading away
	    //See if we should merge any backgrounds
	    for (int k=0;k<NRANGES-1;k++) {
		for (int k2=k+1;k2<NRANGES-1;k2++) {
		    // If we split a N(0,1) distribution at x=0, the two halves have mean 0.8 with std=0.62;  use this below
		    if ( (fabs(range[k][i]-range[k2][i]) < 2*(sigma[k][i]+sigma[k2][i])) && sigma[k][i]<MAXBGSIGMA/2.0 && sigma[k2][i]<MAXBGSIGMA/2.0 && freq[k2][i]>0 && range[k][i]>0 && range[k2][i]>0) {
			dbg("Background.update",2) << "At scan " << i << ", merging bg " << k << " (range=" << range[k][i] << ", sigma=" << sigma[k][i] << ", freq=" << freq[k][i] << ") with "
						   << k2 << " (range=" << range[k2][i] << ", sigma=" << sigma[k2][i] << ", freq=" << freq[k2][i] << ")" << std::endl;
			sigma[k][i]=(sigma[k][i]+sigma[k2][i])/2+fabs(range[k][i]-range[k2][i])/4;
			range[k][i]=(range[k][i]+range[k2][i])/2;
			freq[k][i]=freq[k][i]+freq[k2][i];
			for (int kk=k2;kk<NRANGES-1;kk++)
			    swap(i,kk,kk+1);
			freq[NRANGES-1][i]=0;
			range[NRANGES-1][i]=0;
			sigma[NRANGES-1][i]=MEANBGSIGMA;
			dodump=true;
		    }
		}
	    }
	}
	if (dodump) {
	    for (int k=0;k<NRANGES;k++) {
		dbg("Background.update",3) << "Scan " << i << " " << k << ": " << range[k][i] << " " << sigma[k][i] << " " << freq[k][i] << std::endl;
	    }
	}
    }
}

mxArray *Background::convertToMX() const {
    const char *fieldnames[]={"range","angle","freq","sigma"};
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

    mxArray *pSigma = mxCreateDoubleMatrix(NRANGES,range[0].size(),mxREAL);
    assert(pSigma!=NULL);
    data=mxGetPr(pSigma);
    for (unsigned int i=0;i<range[0].size();i++)
	for (int j=0;j<NRANGES;j++)
	    *data++=sigma[j][i]/UNITSPERM;
    mxSetField(bg,0,"sigma",pSigma);

    if (mxSetClassName(bg,"Background")) {
	fprintf(stderr,"Unable to convert background to a Matlab class\n");
    }
    return bg;
}

// Send /pf/background OSC message
void Background::sendMessages(lo_address &addr, int scanpt) const {
    assert(scanpt>=0 && scanpt<=range[0].size());
    // Send one sample of background as scanpoint#, theta (in degress), range (in meters)
    float angleDeg=scanRes*(scanpt-(range[0].size()-1)/2.0);

    lo_send(addr,"/pf/background","iiff",scanpt,range[0].size(),angleDeg,range[0][scanpt]/UNITSPERM);
}

