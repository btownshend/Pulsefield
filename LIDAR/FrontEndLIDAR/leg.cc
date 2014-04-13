#include <iomanip>
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

Leg::Leg(const Point &pt) {
    position=pt;
    prevPosition=pt;
    posvar=INITIALPOSITIONVAR;
    prevposvar=posvar;
    consecutiveInvisibleCount=0;
}

// Empty constructor used to initialize array, gets overwritten using above ctor subsequently
Leg::Leg() {
    ;
}

std::ostream &operator<<(std::ostream &s, const Leg &l) {
    s << std::fixed << std::setprecision(0)
      << "legpos: " << l.position << "+/-" << sqrt(l.posvar)
      << " maxlike=" << l.maxlike
      << std::setprecision(3);
    return s;
}

void Leg::predict(int nstep, float fps) {
    prevPosition=position;
    position.setX(position.X()+velocity.X()*nstep/fps);
    position.setY(position.Y()+velocity.Y()*nstep/fps);
    prevposvar=posvar;
    posvar=std::min(posvar+DRIFTVAR*nstep,MAXPOSITIONVAR);
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Leg::getObsLike(const Point &pt, int frame,const LegStats &ls) const {
    float dpt=(pt-position).norm();
    float sigma=sqrt(pow(ls.getDiamSigma()/2,2.0)+posvar);
    float like=log(normpdf(dpt, ls.getDiam()/2,sigma)*UNITSPERM);

    // Check if the intersection point would be shadowed by the object (ie the contact is on the wrong side)
    // This is handled by calculating the probabily of the object overlapping the scan line prior to the endpoint.
    float dclr=segment2pt(Point(0.0,0.0),pt,position);
    dbg("Leg.getObsLike",20) << "pt=" << pt << ", leg=" << position << ", pt-leg=" << (pt-position) << "dpt=" << dpt << ", sigma=" << sigma << ", like=" << like << ", dclr=" << dclr << std::endl;
    if (dclr<dpt) {
	float clike1=log(normcdf(dclr,ls.getDiam()/2,sigma));
	float clike2= log(normcdf(dpt,ls.getDiam()/2,sigma));
	like+=clike1-clike2;
	dbg("Leg.getObsLike",20) << "clike=" << clike1 << "-" << clike2 << "=" << clike1-clike2 << ", like=" << like << std::endl;
    }
    like=std::max((float)log(RANDOMPTPROB),like);
    assert(isfinite(like));
    return like;
}

void Leg::update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs, int nstep,float fps, const LegStats &ls, const Leg *otherLeg) {
    // Copy in scanpts
    scanpts=fs;

    // Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
    const float LOGDIAMMU=log(ls.getDiam());
    const float LOGDIAMSIGMA=log(1+ls.getDiamSigma()/ls.getDiam());

    dbg("Leg.update",2) << "Prior: " << *this << std::endl;
    dbg("Leg.update",2) << " fs=" << fs << std::endl;
    
    // Bound search by prior position + 2*sigma(position) + legdiam/2
    float margin;
    margin=2*sqrt(posvar);
    minval=position-margin;
    maxval=position+margin;

    // Make sure any potential measured point is also in the search
    for (unsigned int i=0;i<fs.size();i++) {
	minval=minval.min(vis.getSick()->getPoint(fs[i]));
	maxval=maxval.max(vis.getSick()->getPoint(fs[i]));
    }

    // Increase search by legdiam/2
    minval=minval-ls.getDiam()/2;
    maxval=maxval+ls.getDiam()/2;

    // Initial estimate of grid size
    float step=20;
    likenx=(int)((maxval.X()-minval.X())/step+1.5);
    likeny=(int)((maxval.Y()-minval.Y())/step+1.5);
    if (likenx*likeny > MAXGRIDPTS) {
	step=step*sqrt(likenx*likeny*1.0/MAXGRIDPTS);
	dbg("Leg.update",1) << "Too many grid points (" << likenx << " x " << likeny << ") - increasing stepsize to  " << step << " mm" << std::endl;
    }

    minval.setX(floor(minval.X()/step)*step);
    minval.setY(floor(minval.Y()/step)*step);
    maxval.setX(ceil(maxval.X()/step)*step);
    maxval.setY(ceil(maxval.Y()/step)*step);

    likenx=(int)((maxval.X()-minval.X())/step+1.5);
    likeny=(int)((maxval.Y()-minval.Y())/step+1.5);
    dbg("Leg.update",3) << "Search box = " << minval << " : " << maxval << std::endl;
    dbg("Leg.update",3) << "Search over a " << likenx << " x " << likeny << " grid with " << fs.size() << " points, diam=" << ls.getDiam() << " +/- *" << exp(LOGDIAMSIGMA) << std::endl;

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
	if (sqrt(otherLeg->posvar) > ls.getDiam()+ls.getSepSigma()) {
	    legSepLike = getLegSepLike(ls.getDiam(),ls.getSepSigma(),sqrt(posvar));
	    dbg("Leg.update",3) << "Using simplified model for legsep like since other leg posvar=" << sqrt(otherLeg->posvar) << " > " << ls.getDiam()+ls.getSepSigma() << std::endl;
	    // Can compute just using fixed leg separation of ls.getDiam() since the spread in possible leg separations is not going to make much difference when the position is poorly determined
	    useSepLikeLookup=true;
	}
    }

    // Compute likelihoods based on composite of apriori, contact measurements, and non-hitting rays
    like.resize(likenx*likeny);

    float apriorisigma=sqrt(posvar+SENSORSIGMA*SENSORSIGMA);
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*step;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*step;
	    Point pt(x,y);
	    float adist=(position-pt).norm();

	    // a priori likelihood
	    float apriori=log(normpdf(adist,0,apriorisigma)*UNITSPERM);

	    // Likelihood with respect to unobstructed paths (leg can't be in these paths)
	    float dclr=1e10;
	    for (unsigned int k=0;k<clearsel.size();k++)
		dclr=std::min(dclr,segment2pt(Point(0,0),vis.getSick()->getPoint(clearsel[k]),pt));
	    float clearlike=log(normcdf(log(dclr),LOGDIAMMU,LOGDIAMSIGMA));

	    // Likelihood with respect to positive hits
	    float glike=0;
	    for (unsigned int k=0;k<fs.size();k++) {
		float dpt=(vis.getSick()->getPoint(fs[k])-pt).norm();
		// Take the most likely of the observation being background or this target 
		float obslike=log(normpdf(log(dpt*2),LOGDIAMMU,LOGDIAMSIGMA));
		glike+=std::max(bglike[fs[k]],obslike);
	    }

	    // Likelihood with respect to separation from other leg (if it is set and has low variance)
	    float seplike=0;
	    if (otherLeg!=NULL) {
		// Update likelihood using separtion from other leg
		    float d=(otherLeg->position-pt).norm();
		    if (useSepLikeLookup)
			seplike=legSepLike.lookup(d);
		    else
			seplike=log(normpdf(d,ls.getDiam(),sqrt(otherLeg->posvar+ls.getSepSigma()*ls.getSepSigma()))*UNITSPERM);
		    if (isnan(seplike))
			dbg("Leg.update",3) << "ix=" << ix << ", iy=" << iy << ", d=" << d << ", seplike=" << seplike << std::endl;
	    }

	    like[ix*likeny+iy]=glike+clearlike+apriori+seplike;
	    //assert(isfinite(like[ix*likeny+iy]));
	    dbg("Leg.update",20) << "like[" << ix << "," << iy << "] (x=" << x << ", y=" << y << ") L= " << std::setprecision(1) << like[ix*likeny+iy]  << "  M=" << glike << ", C=" << clearlike << ", A=" << apriori << ", S=" << seplike << std::endl << std::setprecision(3);
	}
    }

    // Find iterator that points to maximum of MLE
    std::vector<float>::iterator mle=std::max_element(like.begin(),like.end());
    maxlike=*mle;

    if (maxlike < MINLIKEFORUPDATES) {
	dbg("Leg.update",1) << "Very unlikely placement: MLE position= " << position << " +/- " << posvar << " with like= " << maxlike << "-- not updating estimates" << std::endl;
	// Don't use this estimate to set the new leg positions, velocities, etc
	return;
    }

    // Use iterator position to figure out location of MLE
    int pos=distance(like.begin(),mle);
    int ix=pos/likeny;
    int iy=pos-ix*likeny;
    position=Point(minval.X()+ix*step,minval.Y()+iy*step);

    // Calculate variance (actual mean-square distance from MLE)
    double var=0;
    double tprob=0;
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*step;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*step;
	    Point pt(x,y);
	    if (like[ix*likeny+iy]<-50)
		// Won't add much!
		continue;
	    double prob=exp(like[ix*likeny+iy]);
	    if (isnan(prob) || !(prob>0))
		dbg("Leg.update",3) << "prob=" << prob << ", like=" << like[ix*likeny+iy] << std::endl;
	    assert(prob>0);
	    var+=prob*pow((pt-position).norm(),2.0);
	    assert(~isnan(var));
	    tprob+=prob;
	}
    }
    assert(tprob>0);
    posvar=var/tprob;
    if (posvar< SENSORSIGMA*SENSORSIGMA) {
	dbg("Leg.update",3) << "Calculated posvar for leg is too low (" << sqrt(posvar) << "), setting to " << SENSORSIGMA << std::endl;
	posvar= SENSORSIGMA*SENSORSIGMA;
    }

    dbg("Leg.update",3) << "Leg MLE position= " << position << " +/- " << sqrt(posvar) << " with like= " << *mle << std::endl;

    if (nstep>0) {
	// Update velocities
	if (fs.size()==0)
	    velocity=velocity*VELDAMPING;
	else
	    velocity=velocity*(1-1/VELUPDATETC)+(position-prevPosition)/(nstep/fps)/VELUPDATETC;

	// Reduce speed if over maximum
	float spd=velocity.norm();
	if (spd>MAXLEGSPEED)
	    velocity=velocity*(MAXLEGSPEED/spd);
    }
}

void Leg::updateVisibility() {
    if (maxlike < MINLIKEFORUPDATES || scanpts.size()==0)
	consecutiveInvisibleCount++;
    else 
	consecutiveInvisibleCount=0;
}

void Leg::updateDiameterEstimates(const Vis &vis, LegStats &ls) const {
    // Update diameter estimate
    if (scanpts.size() >= 5 && (scanpts[scanpts.size()-1]-scanpts[0])==(int)(scanpts.size()-1)) {
	dbg("Leg.updateDiameterEstimates",3) << "Updating leg diameter using " << scanpts.size() << " scan points" << std::endl;
	// check that the leg is clearly in the foreground
	//	if (vis.getSick()->getRange()[ <     }
    }
}

void Leg::sendMessages(lo_address &addr, int frame, int id, int legnum) const {
    if (lo_send(addr,"/pf/leg","iiiiffffffffi",frame,id,legnum,2,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		sqrt(posvar)/UNITSPERM,sqrt(posvar)/UNITSPERM,
		velocity.norm()/UNITSPERM,0.0f,
		velocity.getTheta()*180.0/M_PI,0.0f,
		consecutiveInvisibleCount)<0)
	std::cerr << "Failed send of /pf/leg to OSC port" << std::endl;
}

