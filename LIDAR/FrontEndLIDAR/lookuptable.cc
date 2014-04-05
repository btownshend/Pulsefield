#include <ostream>
#include <math.h>
#include "lookuptable.h"
#include "normal.h"
#include "dbg.h"

LookupTable getLegSepLike(float sepmu,float sepsigma,float possigma) {
    const float LOGSEPMU=log(sepmu);
    const float LOGSEPSIGMA=log(1+sepsigma/sepmu);
    const int nstd=3;    // Run out this far on the individual distributions
    const float step=10;
    float maxtruelegsep=sepmu*exp(nstd*LOGSEPSIGMA);
    float mintruelegsep=sepmu/exp(nstd*LOGSEPSIGMA);
    float maxobslegsep=maxtruelegsep+nstd*possigma;
    int nstep=(int)(maxobslegsep/step+0.5);
    LookupTable tbl(0,maxobslegsep,nstep);
    const float tstep=(log(maxtruelegsep)-log(mintruelegsep))/100;
    for (float logsep=log(mintruelegsep);logsep<log(maxtruelegsep);logsep+=tstep) {
	float truesep=exp(logsep);
	float sepprob=normpdf(logsep,LOGSEPMU,LOGSEPSIGMA);
	//	dbg("getLegSepLike",1) << "p(sep=" << truesep << ")=" << sepprob << std::endl;
	for (int ix=0;ix<nstep;ix++) {
	    float x=ix*step;
	    double prob=normpdf(x-truesep,0,possigma)*1000*sepprob;
	    tbl[ix]+=prob;
	}
    }
    dbg("getLegSepLike",1) << "getLegSepLike(" << sepmu << "," << sepsigma << "," << possigma << "): Size=" << tbl.size() << ", total table coverage=" << tbl.sum()*(step*tstep) << std::endl;
    // Convert to log-like
    for (int i=0;i<tbl.size();i++)
	if (tbl[i]==0)
	    tbl[i]=-1000; // Avoid calling log with zero
	else
	    tbl[i]=log(tbl[i]);

    return tbl;
}
