#include <vector>

#include <string.h>
#include <algorithm>
#include "person.h"
#include "vis.h"
#include "parameters.h"
#include "dbg.h"
#include "normal.h"
#include "lookuptable.h"

static int channeluse[NCHANNELS];

LegStats::LegStats() {
    diam=INITLEGDIAM;
    diamSigma=LEGDIAMSIGMA;
    sep=MEANLEGSEP;
    sepSigma=LEGSEPSIGMA;
    leftness=0.0;
    facing=0.0;
    facingSEM=FACINGSEM;
}

void LegStats::update(const Person &p) {
    Point legdiff=p.getLeg(1).getPosition()-p.getLeg(0).getPosition();
    leftness=leftness*(1-1/LEFTNESSTC)+legdiff.dot(Point(-p.getVelocity().Y(),p.getVelocity().X()))/LEFTNESSTC;
    // TODO: update other stats
}

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

Person::Person(int _id, const Point &leg1, const Point &leg2) {
    id=_id;
    // Assign legs
    legs[0]=Leg(leg1);
    legs[1]=Leg(leg2);

    // Find a free channel, or advance to next one if none free
    channel=0;
    for (int i=0;i<NCHANNELS;i++) {
	if (channeluse[i]==0) {
	    channel=i;
	    break;
	}
    }
    channeluse[channel]++;

    position=(leg1+leg2)/2;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;
}

Person::~Person() {
    channeluse[channel]--;
}

bool Person::isDead() const {
    float visibility = totalVisibleCount*1.0/age;
    bool result = (age<AGETHRESHOLD && visibility<MINVISIBILITY) || (consecutiveInvisibleCount >= INVISIBLEFORTOOLONG);
    if (result) {
	dbg("Person.isDead",2) << " Deleting " << *this << std::endl;
    }
    return result;
}

std::ostream &operator<<(std::ostream &s, const Leg &l) {
    s << std::fixed << std::setprecision(0)
      << "legpos: " << l.position << "+/-" << sqrt(l.posvar)
      << " maxlike=" << l.maxlike;
    return s;
}

std::ostream &operator<<(std::ostream &s, const LegStats &ls) {
    s << std::fixed << std::setprecision(0)
      << "diam:  " << ls.diam
      << ",sep: " << ls.sep;
    return s;
}

std::ostream &operator<<(std::ostream &s, const Person &p) {
    s << "ID " << p.id 
      << std::fixed << std::setprecision(0) 
      << ", position: " << p.position
      << ", leg1: " << p.legs[0]
      << ", leg2: " << p.legs[1]
      << ", " << p.legStats
      << std::setprecision(2)
      << ", vel: " << p.velocity
      << ", age: " << p.age;
    if (p.consecutiveInvisibleCount > 0)
	s << ",invis:" << p.consecutiveInvisibleCount;
    return s;
}

void Leg::predict(int nstep, float fps) {
    prevPosition=position;
    position.setX(position.X()+velocity.X()*nstep/fps);
    position.setY(position.Y()+velocity.Y()*nstep/fps);
    prevposvar=posvar;
    posvar=std::min(posvar+DRIFTVAR*nstep,MAXPOSITIONVAR);
}

void Person::predict(int nstep, float fps) {
    if (nstep==0)
	return;

    for (int i=0;i<2;i++) 
	legs[i].predict(nstep,fps);

    // If one leg is locked down, then the other leg can't vary more than MAXLEGSEP
    for (int i=0;i<2;i++)
	legs[i].posvar=std::min(legs[i].posvar,legs[1-i].posvar+MAXLEGSEP*MAXLEGSEP);

    // Check that they didn't get too close or too far apart
    float legsep=(legs[0].position-legs[1].position).norm();
    if (legsep<legStats.getDiam()-0.1) {
	dbg("Person.predict",1) << "legs are " << legsep << " apart (< " << legStats.getDiam() << "), splitting" << std::endl;
	Point vec;
	if (legsep>0)
	    vec=(legs[0].position-legs[1].position)*(legStats.getDiam()/legsep-1);
	else
	    vec=Point(legStats.getDiam(),0);

	legs[0].position=legs[0].position+vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
	legs[0].position=legs[0].position-vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
    }
    if (legsep>MAXLEGSEP+0.1) {
	dbg("Person.predict",1) << "legs are " << legsep << " apart (> " << MAXLEGSEP << "), moving together" << std::endl;
	Point vec;
	vec=(legs[0].position-legs[1].position)*(MAXLEGSEP/legsep-1);
	legs[0].position=legs[0].position+vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
	legs[0].position=legs[0].position-vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
    }
    position=(legs[0].position+legs[1].position)/2;
    velocity=(legs[0].velocity+legs[1].velocity)/2;
    dbg("Person.predict",2) << "After predict: " << *this << std::endl;
}

// Calculate distance from a line segment define by two points and another point
float  segment2pt(const Point &l1, const Point &l2, const Point &p) {
    Point D=l2-l1;
    Point p1=p-l1;
    float u=D.dot(p)/D.dot(D);
    if (u<=0)
	return p1.norm();
    else if (u>=1)
	return (p-l2).norm();
    else 
	return (p1-D*u).norm();
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Person::getObsLike(const Point &pt, int leg, int frame) const {
    return legs[leg].getObsLike(pt,frame,legStats);
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Leg::getObsLike(const Point &pt, int frame,const LegStats &ls) const {
    float dpt=(pt-position).norm();
    float sigma=sqrt(pow(ls.getDiamSigma()/2,2.0)+posvar);
    float like=log(normpdf(dpt, ls.getDiam()/2,sigma)*UNITSPERM);

    // Check if the intersection point would be shadowed by the object (ie the contact is on the wrong side)
    // This is handled by calculating the probabily of the object overlapping the scan line prior to the endpoint.
    float dclr=segment2pt(Point(0.0,0.0),pt,position);
    dbg("Person.getObsLike",20) << "pt=" << pt << ", leg=" << position << ", pt-leg=" << (pt-position) << "dpt=" << dpt << ", sigma=" << sigma << ", like=" << like << ", dclr=" << dclr << std::endl;
    if (dclr<dpt) {
	float clike1=log(normcdf(dclr,ls.getDiam()/2,sigma));
	float clike2= log(normcdf(dpt,ls.getDiam()/2,sigma));
	like+=clike1-clike2;
	dbg("Person.getObsLike",20) << "clike=" << clike1 << "-" << clike2 << "=" << clike1-clike2 << ", like=" << like << std::endl;
    }
    like=std::max((float)log(RANDOMPTPROB),like);
    assert(isfinite(like));
    return like;
}

void Person::update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs[2], int nstep,float fps) {
    // Need to run 3 passes, leg0,leg1(which by now includes separation likelihoods),and then leg0 again since it was updated during the 2nd iteration due to separation likelihoods
    legs[0].update(vis,bglike,fs[0],nstep,fps,legStats,0);
    legs[1].update(vis,bglike,fs[1],nstep,fps,legStats,&legs[0]);
    if (true) {
	// TODO only if leg[1] adjusted, then do first leg again
	dbg("Person.update",2) << "Re-running update of leg[0] since leg[1] position changed." << std::endl;
	legs[0].update(vis,bglike,fs[0],nstep,fps,legStats,&legs[1]);
	if (legs[0].consecutiveInvisibleCount > 0)
	    legs[0].consecutiveInvisibleCount--;  // Need to back out double increment of this
    }

    if (fs[0].size()==0 && fs[1].size()==0) {
	// Both legs hidden, maintain both at average velocity (already damped by legs.updat())
	legs[0].velocity=(legs[0].velocity+legs[1].velocity)/2.0;
	legs[1].velocity=legs[0].velocity;
    }

    // Average velocity of legs
    velocity=(legs[0].velocity+legs[1].velocity)/2.0;
    assert(isfinite(velocity.X()) && isfinite(velocity.Y()));

    // New position
    position=(legs[0].position+legs[1].position)/2.0;

    // Leftness
    legStats.update(*this);
 
    // Age, visibility counters
    if (legs[0].consecutiveInvisibleCount > 0 && legs[1].consecutiveInvisibleCount > 0)
	consecutiveInvisibleCount++;
    else {
	consecutiveInvisibleCount=0;
	totalVisibleCount++;
    }
    age++;
    dbg("Person.update",2) << "Done: " << *this << std::endl;
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
	    dbg("Person.update",3) << "Using simplified model for legsep like since other leg posvar=" << sqrt(otherLeg->posvar) << " > " << ls.getDiam()+ls.getSepSigma() << std::endl;
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
	    dbg("Leg.update",20) << "like[" << ix << "," << iy << "] (x=" << x << ", y=" << y << ") = " << like[ix*likeny+iy]  << "  M=" << glike << ", C=" << clearlike << ", A=" << apriori << std::endl;
	}
    }

    // Find iterator that points to maximum of MLE
    std::vector<float>::iterator mle=std::max_element(like.begin(),like.end());
    maxlike=*mle;

    if (maxlike < MINLIKEFORUPDATES) {
	dbg("Leg.update",1) << "Very unlikely placement: MLE position= " << position << " +/- " << posvar << " with like= " << maxlike << "-- not updating estimates" << std::endl;
	// Don't use this estimate to set the new leg positions, velocities, etc
	consecutiveInvisibleCount++;
	return;
    }  else if (fs.size() == 0)
	consecutiveInvisibleCount++;  
    else 
	consecutiveInvisibleCount=0;
	

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

void Leg::sendMessages(lo_address &addr, int frame, int id, int legnum) const {
    if (lo_send(addr,"/pf/leg","iiiiffffffffi",frame,id,legnum,2,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		sqrt(posvar)/UNITSPERM,sqrt(posvar)/UNITSPERM,
		velocity.norm()/UNITSPERM,0.0f,
		velocity.getTheta()*180.0/M_PI,0.0f,
		consecutiveInvisibleCount)<0)
	std::cerr << "Failed send of /pf/leg to OSC port" << std::endl;
}

// Send /pf/ OSC messages
void Person::sendMessages(lo_address &addr, int frame, double now) const {
    if (lo_send(addr, "/pf/update","ififfffffiii",frame,now,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		velocity.X()/UNITSPERM,velocity.Y()/UNITSPERM,
		(legStats.getSep()+legStats.getDiam())/UNITSPERM,legStats.getDiam()/UNITSPERM,
		0,0,
		channel) < 0)
	    std::cerr << "Failed send of /pf/update to OSC port" << std::endl;
    if (lo_send(addr, "/pf/body","ififffffffffffffffi",frame,now,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		posvar,posvar,
		velocity.norm()/UNITSPERM,0.0f,
		velocity.getTheta()*180.0/M_PI,0.0f,
		legStats.getFacing(),legStats.getFacingSEM(),
		legStats.getDiam()/UNITSPERM,legStats.getDiamSigma()/UNITSPERM,
		legStats.getSep()/UNITSPERM,legStats.getSepSigma()/UNITSPERM,
		legStats.getLeftness(),
		consecutiveInvisibleCount) < 0)
	    std::cerr << "Failed send of /pf/body to OSC port" << std::endl;
    for (int i=0;i<2;i++)
	legs[i].sendMessages(addr,frame,id,i);
}

void Person::addToMX(mxArray *people, int index) const {
    // const char *fieldnames[]={"id","position","legs","prevlegs","legvelocity","scanpts","posvar","prevposvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount"};
    // Note: for multidimensional arrays, first index changes most rapidly in accessing matlab data
    mxArray *pId = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pId) = id;
    mxSetField(people,index,"id",pId);

    mxArray *pPosition = mxCreateDoubleMatrix(1,2,mxREAL);
    double *data = mxGetPr(pPosition);
    data[0]=position.X()/UNITSPERM;
    data[1]=position.Y()/UNITSPERM;
    mxSetField(people,index,"position",pPosition);

    mxArray *pPosvar = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pPosvar);
    data[0]=legs[0].posvar/1e6;
    data[1]=legs[1].posvar/1e6;
    mxSetField(people,index,"posvar",pPosvar);

    mxArray *pPrevposvar = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pPrevposvar);
    data[0]=legs[0].prevposvar/1e6;
    data[1]=legs[1].prevposvar/1e6;
    mxSetField(people,index,"prevposvar",pPrevposvar);

    mxArray *pVelocity = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pVelocity);
    data[0]=velocity.X()/UNITSPERM;
    data[1]=velocity.Y()/UNITSPERM;
    mxSetField(people,index,"velocity",pVelocity);

    mxArray *pMinval = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pMinval);
    for (int i=0;i<2;i++) 
	*data++=legs[i].minval.X()/UNITSPERM;
    for (int i=0;i<2;i++) 
	*data++=legs[i].minval.Y()/UNITSPERM;
    mxSetField(people,index,"minval",pMinval);

    mxArray *pMaxval = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pMaxval);
    for (int i=0;i<2;i++) 
	*data++=legs[i].maxval.X()/UNITSPERM;
    for (int i=0;i<2;i++) 
	*data++=legs[i].maxval.Y()/UNITSPERM;
    mxSetField(people,index,"maxval",pMaxval);

    mxArray *pScanptsCA=mxCreateCellMatrix(1,2);
    for (int i=0;i<2;i++) {
	mxArray *pScanpts = mxCreateDoubleMatrix(legs[i].scanpts.size(),1,mxREAL);
	data = mxGetPr(pScanpts);
	for (unsigned int j=0;j<legs[i].scanpts.size();j++)
	    *data++=legs[i].scanpts[j]+1;		// Need to change 0-based to 1-based
	mxSetCell(pScanptsCA,i,pScanpts);
    }
    mxSetField(people,index,"scanpts",pScanptsCA);

    mxArray *pLegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pLegs);
    *data++=legs[0].position.X()/UNITSPERM;
    *data++=legs[1].position.X()/UNITSPERM;
    *data++=legs[0].position.Y()/UNITSPERM;
    *data++=legs[1].position.Y()/UNITSPERM;
    mxSetField(people,index,"legs",pLegs);

    mxArray *pLikeCA=mxCreateCellMatrix(1,2);
    for (int i=0;i<2;i++) {
	mxArray *pLike = mxCreateDoubleMatrix(legs[i].likeny,legs[i].likenx,mxREAL);
	assert((int)legs[i].like.size()==legs[i].likenx*legs[i].likeny);
	data = mxGetPr(pLike);
	for (unsigned int j=0;j<legs[i].like.size();j++) 
	    *data++=-legs[i].like[j];   // Use neg loglikes in the matlab version
	mxSetCell(pLikeCA,i,pLike);
    }
    mxSetField(people,index,"like",pLikeCA);

    mxArray *pMaxlike = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pMaxlike) = legs[0].maxlike+legs[1].maxlike;
    mxSetField(people,index,"maxlike",pMaxlike);

    mxArray *pPrevlegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pPrevlegs);
    *data++=legs[0].prevPosition.X()/UNITSPERM;
    *data++=legs[1].prevPosition.X()/UNITSPERM;
    *data++=legs[0].prevPosition.Y()/UNITSPERM;
    *data++=legs[1].prevPosition.Y()/UNITSPERM;
    mxSetField(people,index,"prevlegs",pPrevlegs);

    mxArray *pLegvel = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pLegvel);
    *data++=legs[0].velocity.X()/UNITSPERM;
    *data++=legs[1].velocity.X()/UNITSPERM;
    *data++=legs[0].velocity.Y()/UNITSPERM;
    *data++=legs[1].velocity.Y()/UNITSPERM;
    mxSetField(people,index,"legvelocity",pLegvel);

    mxArray *pLeftness = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pLeftness) = legStats.getLeftness();
    mxSetField(people,index,"leftness",pLeftness);

    mxArray *pLegdiam = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pLegdiam) = legStats.getDiam()/UNITSPERM;
    mxSetField(people,index,"legdiam",pLegdiam);

    mxArray *pAge = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pAge) = age;
    mxSetField(people,index,"age",pAge);

    mxArray *pConsecutiveInvisibleCount = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pConsecutiveInvisibleCount) = consecutiveInvisibleCount;
    mxSetField(people,index,"consecutiveInvisibleCount",pConsecutiveInvisibleCount);

    mxArray *pTotalVisibleCount = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pTotalVisibleCount) = totalVisibleCount;
    mxSetField(people,index,"totalVisibleCount",pTotalVisibleCount);
}
