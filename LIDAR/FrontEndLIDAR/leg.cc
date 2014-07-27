#include <iomanip>
#include <cmath>
#include <math.h>
#include <assert.h>
#include <algorithm>

#include "lo/lo.h"
#include "legstats.h"
#include "leg.h"
#include "parameters.h"
#include "dbg.h"
#include "normal.h"
#include "vis.h"
#include "lookuptable.h"

static const bool USEMLE=false;  // True to use MLE from likelihood grid; otherwise use mean

Leg::Leg(const Point &pt) {
    position=pt;
    predictedPosition=pt;
    posvar=INITIALPOSITIONVAR;
    prevposvar=posvar;
    consecutiveInvisibleCount=0;
    velocity=Point(0,0);

    like.clear();
    scanpts.clear();
    maxlike=nan("");
    likenx=0;
    likeny=0;
    minval=Point(nan(""),nan(""));
    maxval=Point(nan(""),nan(""));

    //static const float weights[]={0.8234,0.0699,0.8303,0.0862,0.7780,0.1283,.7262,.1585,.6818,.1866,.6289,.2169};  // Need scaling by nweight/2 
    //static const int nweights=sizeof(weights)/sizeof(weights[0]);
    //predictWeights.assign(weights,weights+nweights);

    const int nweights=50;
    predictWeights.resize(nweights);

    const int strideFrames=61;   // Number of frames for a complete stride

    // Initial weight is best predictor of a sine wave offset by 1/strideFrames of a cycle
    predictWeights[0]=cos(2*M_PI/strideFrames);
    predictWeights[1]=sin(2*M_PI/strideFrames);
    const float totalDamping=0.9864;
    // damp things so legs reach equal predicted velocity in 1/4 stride
    const float sameDamping=pow(2.0,-1.0f/(strideFrames/4))*totalDamping;
    const float desiredTotal=0.85;
    float sum=predictWeights[0]+predictWeights[1];
    for (int i=2;i<nweights;i+=2) {
	predictWeights[i]=predictWeights[i-2]*sameDamping;
	predictWeights[i+1]=(predictWeights[i-2]+predictWeights[i-1])*totalDamping-predictWeights[i];
	if (predictWeights[i+1]>predictWeights[i]) {
	    predictWeights[i]=(predictWeights[i]+predictWeights[i+1])/2;
	    predictWeights[i+1]=predictWeights[i];
	}
	sum+=predictWeights[i]+predictWeights[i+1];
    }
    dbg("Leg.Leg",1) << "Damping = " << totalDamping << ", " << sameDamping << "  Weights=[";
    for (int i=0;i<nweights;i+=2) {
	predictWeights[i] *= desiredTotal/sum;
	predictWeights[i+1] *= desiredTotal/sum;
	dbgn("Leg.Leg",1) << "(" << predictWeights[i]*(nweights/2) << "," << predictWeights[i+1]*(nweights/2) << ") ";
    }
    dbgn("Leg.Leg",1) << "]" << std::endl;
}

// Empty constructor used to initialize array, gets overwritten using above ctor subsequently
Leg::Leg() {
    ;
}

std::ostream &operator<<(std::ostream &s, const Leg &l) {
    s << std::fixed << std::setprecision(0)
      << "pos: " << l.position << "+/-" << sqrt(l.posvar)
      << ", vel: " << l.velocity
      << ", maxlike=" << l.maxlike
      << std::setprecision(3);
    return s;
}

// Predict next leg position from current one
// Parameters from optimalveldamping2.m
void Leg::predict(const Leg &otherLeg) {
    Point newDelta(0,0);
    float rmse;
    for (int i=0;i<predictWeights.size();i+=2)
	newDelta=newDelta+predictWeights[i]*getPriorDelta(i/2+1);
    for (int i=1;i<predictWeights.size();i+=2)
	newDelta=newDelta+predictWeights[i]*otherLeg.getPriorDelta((i+1)/2);
    //    rmse=newDelta.norm()*0.16+10;
    rmse=newDelta.norm()*0.08+7;

    dbg("Leg.predict",5) << "newDelta=" << newDelta << ", rmse=" << rmse << std::endl;

    position=position+newDelta;
    prevposvar=posvar;
    posvar=std::min(posvar+rmse*rmse,MAXPOSITIONVAR);
    predictedPosition=position;   // Save this before applying measurements for subsequent analyses

    // Clear out variables that are no longer valid
    like.clear();
    scanpts.clear();
    maxlike=nan("");
    likenx=0;
    likeny=0;
    minval=Point(nan(""),nan(""));
    maxval=Point(nan(""),nan(""));
}

void Leg::savePriorPositions() {
    priorPositions.push_back(position);
    // Keep the size under control
    if (priorPositions.size()>1000)
	priorPositions.erase(priorPositions.begin(),priorPositions.begin()+500);
}

Point Leg::getPriorDelta(int n) const {
    assert(n>0);
    if (n+1  > priorPositions.size())
	return Point(0,0);
    return priorPositions[priorPositions.size()-n]-priorPositions[priorPositions.size()-n-1];
}

Point Leg::getPriorPosition(int n) const {
    assert(n>0);
    if (n  > priorPositions.size())
	return Point(0,0);
    return priorPositions[priorPositions.size()-n];
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Leg::getObsLike(const Point &pt, int frame,const LegStats &ls) const {
    float dpt=(pt-position).norm();
    float sigma=sqrt(pow(ls.getDiamSigma()/2,2.0)+posvar);  // hack: use positition sigma inflated by leg diam variance
    // float like=normlike(dpt, ls.getDiam()/2,sigma);
    Point delta=pt-position;
    Point diamOffset=delta/delta.norm()*ls.getDiam()/2;
    float like=normlike(delta.X(),diamOffset.X(),sigma)+normlike(delta.Y(),diamOffset.Y(),sigma);
    // And check the likelihood that the echo is in front of the true leg position
    float frontlike=log(1-normcdf(pt.norm(),position.norm(),sqrt(posvar)));
    dbg("Leg.getObsLike",20) << "pt=" << pt << ", leg=" << position << ", pt-leg=" << (pt-position) << ", diam=" << ls.getDiam() << ", dpt=" << dpt << ", sigma=" << sigma << ", like=" << like << ", frontlike=" << frontlike << std::endl;
    like+=frontlike;
    like=std::max((float)log(RANDOMPTPROB),like);
    assert(std::isfinite(like));
    return like;
}

void Leg::update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs,const LegStats &ls, const Leg *otherLeg) {
    // Copy in scanpts
    scanpts=fs;

    // Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
    const float LOGDIAMMU=log(ls.getDiam());
    const float LOGDIAMSIGMA=log(1+ls.getDiamSigma()/ls.getDiam());

    dbg("Leg.update",5) << "prior: " << *this << std::endl;
    dbg("Leg.update",5) << " fs=" << fs << std::endl;
    

    // Find the rays that will hit this box
    float theta[4];
    theta[0]=Point(maxval.X(),minval.Y()).getTheta();
    theta[1]=Point(maxval.X(),maxval.Y()).getTheta();
    theta[2]=Point(minval.X(),minval.Y()).getTheta();
    theta[3]=Point(minval.X(),maxval.Y()).getTheta();
    float mintheta=std::min(std::min(theta[0],theta[1]),std::min(theta[2],theta[3]));
    float maxtheta=std::max(std::max(theta[0],theta[1]),std::max(theta[2],theta[3]));
    std::vector<int> clearsel;
    dbg("Leg.update",3) << "Clear paths for " << mintheta*180/M_PI << "-" << maxtheta*180/M_PI <<  " degrees:   ";
    for (unsigned int i=0;i<vis.getSick()->getNumMeasurements();i++) {
	float angle=vis.getSick()->getAngleRad(i);
	if (angle>=mintheta && angle<=maxtheta) {
	    clearsel.push_back(i);
	    dbgn("Leg.update",3) << i << ",";
	}
    }
    dbgn("Leg.update",3) << std::endl;


    bool useSepLikeLookup=false;
    LookupTable legSepLike;

    if (otherLeg!=NULL) {
	// Calculate separation likelihood using other leg at MLE with computed variance
	if (sqrt(otherLeg->posvar) > ls.getSep()+ls.getSepSigma()) {
	    legSepLike = getLegSepLike(ls.getSep(),ls.getSepSigma(),sqrt(posvar));
	    dbg("Leg.update",3) << "Using simplified model for legsep like since other leg posvar=" << sqrt(otherLeg->posvar) << " > " << ls.getSep()+ls.getSepSigma() << std::endl;
	    // Can compute just using fixed leg separation of ls.getSep() since the spread in possible leg separations is not going to make much difference when the position is poorly determined
	    useSepLikeLookup=true;
	}
    }

    // Compute likelihoods based on composite of apriori, contact measurements, and non-hitting rays
    like.resize(likenx*likeny);

    float apriorisigma=sqrt(posvar+SENSORSIGMA*SENSORSIGMA);
    float stepx=(maxval.X()-minval.X())/(likenx-1);
    float stepy=(maxval.Y()-minval.Y())/(likeny-1);
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*stepx;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*stepy;
	    Point pt(x,y);
	    // float adist=(position-pt).norm();
	    
	    // a priori likelihood
	    // float apriori=normlike(adist,0,apriorisigma);  // WRONG computation!

	    Point delta=position-pt;
	    float apriori=normlike(delta.X(),0,apriorisigma)+normlike(delta.Y(),0,apriorisigma);   // Assume apriorsigma applies in both directions, no covariance

	    // Likelihood with respect to unobstructed paths (leg can't be in these paths)
	    float dclr=1e10;
	    for (unsigned int k=0;k<clearsel.size();k++)
		dclr=std::min(dclr,segment2pt(Point(0,0),vis.getSick()->getPoint(clearsel[k]),pt));
	    float clearlike=log(normcdf(log(dclr*2),LOGDIAMMU,LOGDIAMSIGMA));

	    // Likelihood with respect to positive hits
	    float glike=0;
	    float bgsum=0;
	    for (unsigned int k=0;k<fs.size();k++) {
		float dpt=(vis.getSick()->getPoint(fs[k])-pt).norm();
		// Scale it so it is a density per meter in the area of the mean
		float obslike=log(normpdf(log(dpt*2),LOGDIAMMU,LOGDIAMSIGMA)*(UNITSPERM/ls.getDiam()));
		// Take the most likely of the observation being background or this target 
		glike+=std::max(bglike[fs[k]],obslike);
		bgsum+=bglike[fs[k]];
	    }

	    // Likelihood with respect to separation from other leg (if it is set and has low variance)
	    float seplike=0;
	    if (otherLeg!=NULL) {
		// Update likelihood using separation from other leg
		    float d=(otherLeg->position-pt).norm();
		    if (d>MAXLEGSEP*2)
			seplike=-1000;
		    else
			if (useSepLikeLookup)
			seplike=legSepLike.lookup(d);
		    else
			seplike=log(ricepdf(d,ls.getSep(),sqrt(otherLeg->posvar+ls.getSepSigma()*ls.getSepSigma()))*UNITSPERM);
		    if (std::isnan(seplike))
			dbg("Leg.update",3) << "ix=" << ix << ", iy=" << iy << ", d=" << d << ", uselookup=" << useSepLikeLookup << ", sep=" << ls.getSep() << ", sigma=" << (sqrt(otherLeg->posvar+ls.getSepSigma()*ls.getSepSigma()))  << ", seplike=" << seplike << std::endl;
	    }

	    like[ix*likeny+iy]=glike+clearlike+seplike+apriori;
		
	    //assert(isfinite(like[ix*likeny+iy]));
	    dbg("Leg.update",20) << "like[" << ix << "," << iy << "] (x=" << x << ", y=" << y << ") L= " << std::setprecision(1) << like[ix*likeny+iy]  << "  M=" << glike << ",BG=" << bgsum << ", C=" << clearlike << ", A=" << apriori << ", S=" << seplike << std::endl << std::setprecision(3);
	}
    }

    // Find iterator that points to maximum of MLE
    std::vector<float>::iterator mle=std::max_element(like.begin(),like.end());
    maxlike=*mle;
    // Use iterator position to figure out location of MLE
    int pos=distance(like.begin(),mle);
    int ix=pos/likeny;
    int iy=pos-ix*likeny;
    Point mlepos(minval.X()+ix*stepx,minval.Y()+iy*stepy);

    if (maxlike < MINLIKEFORUPDATES) {
	dbg("Leg.update",1) << "Very unlikely placement: MLE position= " << mlepos <<  " with like= " << maxlike << "-- not updating estimates, leaving leg at " << position <<"+=" << sqrt(posvar) <<  std::endl;
	// Don't use this estimate to set the new leg positions, velocities, etc
	return;
    }


    // Find mean location by averaging over grid
    Point sum(0,0);
    double tprob=0;
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*stepx;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*stepy;
	    Point pt(x,y);
	    if (like[ix*likeny+iy]-*mle<-12)   
		// Won't add much!  Less than likenx*likeny*exp(-12)
		continue;
	    double prob=exp(like[ix*likeny+iy]-*mle);
	    if (std::isnan(prob) || !(prob>0))
		dbg("Leg.update",3) << "prob=" << prob << ", like=" << like[ix*likeny+iy] << std::endl;
	    assert(prob>0);
	    sum=sum+pt*prob;
	    tprob+=prob;
	}
    }
    assert(tprob>0);  // Since MLE was found, there must be some point that works
    Point mean=sum/tprob;

    dbg("Leg.update",3) << "Got MLE=" << mlepos << " with like=" << *mle << ", mean position=" << mean << ", tprob=" << tprob << std::endl;
    Point newposition;
    if (USEMLE)
	newposition=mlepos;
    else
	newposition=mean;
    if (velocity.norm() <= STATIONARYVELOCITY ) {
	// Not moving, keep position a little more stable
	dbg("Leg.update",3) << "Speed = " << velocity.norm() << " mm/frame, stabilizing position" << std::endl;
	Point delta=newposition-position;
	position=position+delta/10;
    } else {
	dbg("Leg.update",3) << "Speed = " << velocity.norm() << " mm/frame, not stabilizing position" << std::endl;
	position=newposition;
    }
    dbg("Leg.update",3) << "Target position=" << newposition << ", updated position=" << position << std::endl;

    // Calculate variance (actual mean-square distance from MLE)
    double var=0;
    tprob=0;
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*stepx;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*stepy;
	    Point pt(x,y);
	    if (like[ix*likeny+iy]-*mle<-12)
		// Won't add much!
		continue;
	    double prob=exp(like[ix*likeny+iy]-*mle);
	    if (std::isnan(prob) || !(prob>0))
		dbg("Leg.update",3) << "prob=" << prob << ", like=" << like[ix*likeny+iy] << std::endl;
	    assert(prob>0);
	    var+=prob*pow((pt-position).norm(),2.0);
	    assert(~std::isnan(var));
	    tprob+=prob;
	}
    }
    assert(tprob>0);
    posvar=var/tprob;
    if (posvar< SENSORSIGMA*SENSORSIGMA) {
	dbg("Leg.update",3) << "Calculated posvar for leg is too low (" << sqrt(posvar) << "), setting to " << SENSORSIGMA << std::endl;
	posvar= SENSORSIGMA*SENSORSIGMA;
    }

    dbg("Leg.update",3) << "Leg position= " << position << " +/- " << sqrt(posvar) << " with like= " << *mle << std::endl;
}

void Leg::updateVelocity(int nstep, float fps,Point otherLegVelocity) {
    // Update velocities
    velocity=velocity*(1-1.0f/VELUPDATETC)+getPriorDelta(1)*fps/VELUPDATETC;

    // Reduce speed if over maximum
    float spd=velocity.norm();
    if (spd>MAXLEGSPEED)
	velocity=velocity*(MAXLEGSPEED/spd);
}

void Leg::updateVisibility() {
    if (maxlike < MINLIKEFORUPDATES || scanpts.size()==0)
	consecutiveInvisibleCount++;
    else 
	consecutiveInvisibleCount=0;
}

void Leg::updateDiameterEstimates(const Vis &vis, LegStats &ls) const {
    // Update diameter estimate if we have a contiguous set of hits of adequate length
    if (scanpts.size() >= 5 && (scanpts[scanpts.size()-1]-scanpts[0])==(int)(scanpts.size()-1)) {
	dbg("Leg.updateDiameterEstimates",3) << "Updating leg diameter using " << scanpts.size() << " scan points" << std::endl;
	// check that the leg is clearly in the foreground
	const unsigned int *srange=vis.getSick()->getRange(0);
	unsigned int firstScan=scanpts[0];
	if (firstScan>0 && srange[firstScan-1]<srange[firstScan]+4*ls.getDiam()) {
	    dbg("Leg.updateDiameterEstimates",3) << "Left edge too close to background: " << srange[firstScan-1] << "<" << srange[firstScan]+4*ls.getDiam() << std::endl;
	    return;
	}
	unsigned int lastScan=scanpts[scanpts.size()-1];
	if (lastScan<vis.getSick()->getNumMeasurements()-1 && srange[lastScan+1]<srange[lastScan]+4*ls.getDiam()) {
	    dbg("Leg.updateDiameterEstimates",3) << "Right edge too close to background: " << srange[lastScan+1] << "<" << srange[lastScan]+4*ls.getDiam() <<  std::endl;
	    return;
	}
	float diamEstimate = (vis.getSick()->getPoint(lastScan)-vis.getSick()->getPoint(firstScan)).norm();
	ls.updateDiameter(diamEstimate,diamEstimate/scanpts.size());   // TODO- improve estimate
    }
}

void Leg::sendMessages(lo_address &addr, int frame, int id, int legnum) const {
    if (lo_send(addr,"/pf/leg","iiiiffffffffi",frame,id,legnum,2,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		sqrt(posvar)/UNITSPERM,sqrt(posvar)/UNITSPERM,
		velocity.norm()/UNITSPERM,0.0f,
		velocity.getTheta()*180.0/M_PI,0.0f,
		consecutiveInvisibleCount)<0)
	std::cerr << "Failed send of /pf/leg to " << lo_address_get_url(addr) << std::endl;
}

