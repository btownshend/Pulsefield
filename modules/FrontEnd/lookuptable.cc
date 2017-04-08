#include <ostream>
#include <math.h>
#include <assert.h>
#include "lookuptable.h"
#include "normal.h"
#include "dbg.h"
#include "parameters.h"

LookupTable getLegSepLike(float sepmu,float sepsigma,float possigma) {
    assert(sepsigma>0);
    assert(possigma>0);
    const float LOGSEPMU=log(sepmu);
    const float LOGSEPSIGMA=log(1+sepsigma/sepmu);
    static const float nstd=2.5;    // Run out this far on the individual distributions - should give 0.9752 coverage
    const float maxtruelegsep=sepmu*exp(nstd*LOGSEPSIGMA);
    const float mintruelegsep=sepmu/exp(nstd*LOGSEPSIGMA);
    const float maxobslegsep=std::max(maxtruelegsep,sepmu+nstd*possigma);
    float step=10;  // Step size in mm
    const int nstep=std::min(50,(int)(maxobslegsep/step+0.5));
    step=maxobslegsep/nstep;
    LookupTable tbl(0,maxobslegsep,nstep);
    static const int ntstep=30;
    const float tstep=(log(maxtruelegsep)-log(mintruelegsep))/ntstep;
    for (float logsep=log(mintruelegsep);logsep<log(maxtruelegsep);logsep+=tstep) {
	float truesep=exp(logsep);
	float sepprob=normpdf(logsep,LOGSEPMU,LOGSEPSIGMA);
	//	dbg("getLegSepLike",1) << "p(sep=" << truesep << ")=" << sepprob << std::endl;
	for (int ix=0;ix<nstep;ix++) {
	    float x=ix*step;
	    double prob=ricepdf(x,truesep,possigma)*UNITSPERM*sepprob;
	    tbl[ix]+=prob;
	}
    }
    dbg("getLegSepLike",5) << "getLegSepLike(" << sepmu << "," << sepsigma << "," << possigma << "): Size=" << tbl.size() << ", with " << nstep*ntstep << " inner loops, total table coverage=" << tbl.sum()*(step*tstep)/UNITSPERM << ", ends=" << tbl[0]*(step*tstep)/UNITSPERM << "," << tbl[tbl.size()-1]*(step*tstep)/UNITSPERM << std::endl;
    // Convert to log-like
    for (unsigned int i=0;i<tbl.size();i++)
	if (tbl[i]==0)
	    tbl[i]=-1000; // Avoid calling log with zero
	else
	    tbl[i]=log(tbl[i]);

    return tbl;
}
