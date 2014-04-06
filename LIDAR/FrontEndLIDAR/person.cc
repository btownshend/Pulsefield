#include <vector>
#include <string.h>
#include <algorithm>
#include "person.h"
#include "vis.h"
#include "parameters.h"
#include "likelihood.h"
#include "dbg.h"
#include "normal.h"
#include "lookuptable.h"

static const int NCHANNELS=16;
static int channeluse[NCHANNELS];

void Person::init(int _id, const Point &leg1, const Point &leg2) {
    id=_id;
    // Find a free channel, or advance to next one if none free
    channel=0;
    for (int i=0;i<NCHANNELS;i++) {
	if (channeluse[i]==0) {
	    channel=i;
	    break;
	}
    }
    channeluse[channel]++;

    legdiam=INITLEGDIAM;
    legs[0]=leg1;
    legs[1]=leg2;
    position=(legs[0]+legs[1])/2;
    for (int i=0;i<2;i++) {
	prevlegs[i]=legs[i];
	posvar[i]=INITIALPOSITIONVAR;
	prevposvar[i]=posvar[i];
    }
    leftness=0.0;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;
}

Person::Person(int _id, const Point &leg1, const Point &leg2) {
    init(_id,leg1,leg2);
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

std::ostream &operator<<(std::ostream &s, const Person &p) {
    s << "ID " << p.id 
      << std::fixed << std::setprecision(0) 
      << ", position: " << p.position
      << ", legs: " << p.legs[0] << "+/-" << sqrt(p.posvar[0])
      << ", " << p.legs[1] << "+/-" << sqrt(p.posvar[1])
      << ", diam: " << p.legdiam
      << std::setprecision(2)
      << ", vel: " << p.velocity
      << ", like: " << p.maxlike
      << ", age: " << p.age;
    return s;
}

void Person::predict(int nstep, float fps) {
    if (nstep==0)
	return;

    for (int i=0;i<2;i++) {
	prevlegs[i]=legs[i];
	legs[i].setX(legs[i].X()+legvelocity[i].X()*nstep/fps);
	legs[i].setY(legs[i].Y()+legvelocity[i].Y()*nstep/fps);

	prevposvar[i]=posvar[i];
	posvar[i]+=DRIFTVAR*nstep;
    }
    // If one leg is locked down, then the other leg can't vary more than MAXLEGSEP
    for (int i=0;i<2;i++)
	posvar[i]=std::min(posvar[i],posvar[1-i]+MAXLEGSEP*MAXLEGSEP);
    // Check that they didn't get too close or too far apart
    float legsep=(legs[0]-legs[1]).norm();
    if (legsep<legdiam-0.1) {
	dbg("Person.predict",1) << "legs are " << legsep << " apart (< " << legdiam << "), splitting" << std::endl;
	Point vec;
	if (legsep>0)
	    vec=(legs[0]-legs[1])*(legdiam/legsep-1);
	else
	    vec=Point(legdiam,0);

	legs[0]=legs[0]+vec*(posvar[0]/(posvar[0]+posvar[1]));
	legs[1]=legs[1]-vec*(posvar[1]/(posvar[0]+posvar[1]));
    }
    if (legsep>MAXLEGSEP+0.1) {
	dbg("Person.predict",1) << "legs are " << legsep << " apart (> " << MAXLEGSEP << "), moving together" << std::endl;
	Point vec;
	vec=(legs[0]-legs[1])*(MAXLEGSEP/legsep-1);
	legs[0]=legs[0]+vec*(posvar[0]/(posvar[0]+posvar[1]));
	legs[1]=legs[1]-vec*(posvar[1]/(posvar[0]+posvar[1]));
    }
    position.setX((legs[0].X()+legs[1].X())/2.0);
    position.setY((legs[0].Y()+legs[1].Y())/2.0);
    velocity.setX((legvelocity[0].X()+legvelocity[1].X())/2.0);
    velocity.setY((legvelocity[0].Y()+legvelocity[1].Y())/2.0);
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
    float dpt=(pt-legs[leg]).norm();
    float sigma=sqrt(pow(LEGDIAMSTD/2,2.0)+posvar[leg]);
    float like=log(normpdf(dpt, legdiam/2,sigma)*1000);
    // Check if the intersection point would be shadowed by the object (ie the contact is on the wrong side)
    // This is handled by calculating the probabily of the object overlapping the scan line prior to the endpoint.
    float dclr=segment2pt(Point(0.0,0.0),pt,legs[leg]);
    dbg("Person.getObsLike",20) << "pt=" << pt << ", leg=" << legs[leg] << ", pt-leg=" << (pt-legs[leg]) << "dpt=" << dpt << ", sigma=" << sigma << ", like=" << like << ", dclr=" << dclr << std::endl;
    if (dclr<dpt) {
	float clike1=log(normcdf(dclr,legdiam/2,sigma));
	float clike2= log(normcdf(dpt,legdiam/2,sigma));
	like+=clike1-clike2;
	dbg("Person.getObsLike",20) << "clike=" << clike1 << "-" << clike2 << "=" << clike1-clike2 << ", like=" << like << std::endl;
    }
    return like;
}

void Person::update(const Vis &vis, const std::vector<int> fs[2], int nstep,float fps) {
    // Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
    const float LOGDIAMMU=log(legdiam);
    const float LOGDIAMSIGMA=log(1+LEGDIAMSTD/legdiam);

    float step=20;

    dbg("Person.update",2) << "Initial leg positions " << legs[0] << " and " << legs[1] << " fs=" << fs[0] << " , " << fs[1] << std::endl;
    
    // Bound search by prior position + 2*sigma(position) + legdiam/2
    float margin[2];
    margin[0]=2*sqrt(posvar[0]);
    margin[1]=2*sqrt(posvar[1]);
    minval=(legs[0]-margin[0]).min(legs[1]-margin[1]);
    maxval=(legs[0]+margin[0]).max(legs[1]+margin[1]);
    dbg("Person.update",3) << "Search box = " << minval << " : " << maxval << std::endl;

    // Make sure any potential measured point is also in the search
    for (unsigned int i=0;i<fs[0].size();i++) {
	minval=minval.min(vis.getSick()->getPoint(fs[0][i]));
	maxval=maxval.max(vis.getSick()->getPoint(fs[0][i]));
    }
    for (unsigned int i=0;i<fs[1].size();i++) {
	minval=minval.min(vis.getSick()->getPoint(fs[1][i]));
	maxval=maxval.max(vis.getSick()->getPoint(fs[1][i]));
    }
    dbg("Person.update",3) << "Search box = " << minval << " : " << maxval << std::endl;

    // Increase search by legdiam/2
    minval=minval-legdiam/2;
    maxval=maxval+legdiam/2;
    dbg("Person.update",3) << "Search box = " << minval << " : " << maxval << std::endl;

    minval.setX(floor(minval.X()/step)*step);
    minval.setY(floor(minval.Y()/step)*step);
    maxval.setX(ceil(maxval.X()/step)*step);
    maxval.setY(ceil(maxval.Y()/step)*step);
    dbg("Person.update",3) << "Search box = " << minval << " : " << maxval << std::endl;

    // Find the rays that will hit this box
    float theta[4];
    theta[0]=Point(maxval.X(),minval.Y()).getTheta();
    theta[1]=Point(maxval.X(),maxval.Y()).getTheta();
    theta[2]=Point(minval.X(),minval.Y()).getTheta();
    theta[3]=Point(minval.X(),maxval.Y()).getTheta();
    float mintheta=std::min(std::min(theta[0],theta[1]),std::min(theta[2],theta[3]));
    float maxtheta=std::max(std::max(theta[0],theta[1]),std::max(theta[2],theta[3]));
    std::vector<int> clearsel;
    dbg("Person.update",2) << "Clear paths for " << mintheta*180/M_PI << "-" << maxtheta*180/M_PI <<  " degrees:   ";
    for (unsigned int i=0;i<vis.getSick()->getNumMeasurements();i++) {
	float angle=vis.getSick()->getAngleRad(i);
	if (angle>=mintheta && angle<=maxtheta) {
	    clearsel.push_back(i);
	    dbgn("Person.update",2) << i << ",";
	}
    }
    dbgn("Person.update",2) << std::endl;

    likenx=(int)((maxval.X()-minval.X())/step+1.5);
    likeny=(int)((maxval.Y()-minval.Y())/step+1.5);
    dbg("Person.update",2) << "Search over a " << likenx << " x " << likeny << " grid with " << fs[0].size() << "," << fs[1].size() << " points/leg, diam=" << legdiam << " +/- *" << exp(LOGDIAMSIGMA) << std::endl;

    // Compute likelihoods based on composite of apriori, contact measurements, and non-hitting rays
    like[0].resize(likenx*likeny);
    like[1].resize(likenx*likeny);
    for (int i=0;i<2;i++) {
	float apriorisigma=sqrt(posvar[i]+SENSORSIGMA*SENSORSIGMA);
	for (int ix=0;ix<likenx;ix++) {
	    float x=minval.X()+ix*step;
	    for (int iy=0;iy<likeny;iy++) {
		float y=minval.Y()+iy*step;
		Point pt(x,y);
		float adist=(legs[i]-pt).norm();
		float apriori=log(normpdf(adist,0,apriorisigma)*1000);
		float dclr=1e10;
		for (unsigned int k=0;k<clearsel.size();k++)
		    dclr=std::min(dclr,segment2pt(Point(0,0),vis.getSick()->getPoint(clearsel[k]),pt));
		float clearlike=log(normcdf(log(dclr),LOGDIAMMU,LOGDIAMSIGMA));
		float glike=0;
		for (unsigned int k=0;k<fs[i].size();k++) {
		    float dpt=(vis.getSick()->getPoint(fs[i][k])-pt).norm();
		    glike+=log(normpdf(log(dpt*2),LOGDIAMMU,LOGDIAMSIGMA));
		}
		like[i][ix*likeny+iy]=glike+clearlike+apriori;
		if (like[i][ix*likeny+iy] > -20)
		    dbg("Person.update",5) << "like[" << i << "][" << ix*likeny+iy << "] (x=" << x << ", y=" << y << ") = " << like[i][ix*likeny+iy]  << "  M=" << glike << ", C=" << clearlike << ", A=" << apriori << std::endl;
	    }
	}
    }

    // Run computations based on grid of likelihoods
    // Need to run 3 passes, leg0,leg1(which by now includes separation likelihoods),and then leg0 again since it was updated during the 2nd iteration due to separation likelihoods
    std::vector<float>::iterator mle[2];  // Maximum-likelihood estimaters
    for (int pass=0;pass<3;pass++) {
	int i=pass%2;
	mle[i]=std::max_element(like[i].begin(),like[i].end());
	if (*mle[i] < -30) {
	    dbg("Person.update",1) << "Very unlikely placement: leg[" << i << "]  MLE position= " << legs[i] << " +/- " << posvar[i] << " with like= " << *mle[i] << "-- not refining variance estimate" << std::endl;
	    // Don't use this estimate to set the separation likelihood of the other leg
	    continue;
	}
	int pos=distance(like[i].begin(),mle[i]);
	int ix=pos/likeny;
	int iy=pos-ix*likeny;
	legs[i]=Point(minval.X()+ix*step,minval.Y()+iy*step);

	// Calculate variance (actual mean-square distance from MLE)
	double var=0;
	double tprob=0;
	for (int ix=0;ix<likenx;ix++) {
	    float x=minval.X()+ix*step;
	    for (int iy=0;iy<likeny;iy++) {
		float y=minval.Y()+iy*step;
		Point pt(x,y);
		if (like[i][ix*likeny+iy]<-50)
		    // Won't add much!
		    continue;
		double prob=exp(like[i][ix*likeny+iy]);
		if (isnan(prob) || !(prob>0))
		    dbg("Person.update",3) << "prob=" << prob << ", like=" << like[i][ix*likeny+iy] << std::endl;
		assert(prob>0);
		var+=prob*pow((pt-legs[i]).norm(),2.0);
		assert(~isnan(var));
		tprob+=prob;
	    }
	}
	assert(tprob>0);
	posvar[i]=var/tprob;
	dbg("Person.update",3) << "Leg[" << i << "]  MLE position= " << legs[i] << " +/- " << posvar[i] << " with like= " << *mle[i] << std::endl;

	if (pass==2) 
	    // Don't add the seplike a second time!
	    break;

	// Calculate separation likelihood using other leg at MLE with computed variance
	LookupTable legSepLike = getLegSepLike(MEANLEGSEP,LEGSEPSTD,sqrt(posvar[i]));

	for (int ix=0;ix<likenx;ix++) {
	    float x=minval.X()+ix*step;
	    for (int iy=0;iy<likeny;iy++) {
		float y=minval.Y()+iy*step;
		Point pt(x,y);
		float d=(legs[i]-pt).norm();
		float seplike=legSepLike.lookup(d);
		if (isnan(seplike))
		    dbg("Person.update",3) << "ix=" << ix << ", iy=" << iy << ", d=" << d << ", seplike=" << seplike << std::endl;
		like[1-i][ix*likeny+iy]+=seplike;   // It is the likelihood of the OTHER leg that we are modifying here
	    }
	}
    }

    // Overall likelihood
    maxlike = *mle[0]+*mle[1];

    // Copy in scanpts
    scanpts[0]=fs[0];
    scanpts[1]=fs[1];

    // Update velocities
    if (nstep>=0) {
	if (fs[0].size()==0 && fs[1].size()==0) {
	    // Both legs hidden, maintain both at average velocity (damped)
	    legvelocity[0]=(legvelocity[0]+legvelocity[1])/2.0*VELDAMPING;
	    legvelocity[1]=legvelocity[0];
	} else {
	    for (int i=0;i<2;i++) {
		if (fs[i].size()==0)
		    legvelocity[i]=legvelocity[i]*VELDAMPING;
		else
		    legvelocity[i]=legvelocity[i]*(1-1/VELUPDATETC)+(legs[0]-prevlegs[0])/(nstep/fps)/VELUPDATETC;
	    }
	}
	// Reduce speed if over maximum
	for (int k=0;k<2;k++) {
	    float spd=legvelocity[k].norm();
	    if (spd>MAXLEGSPEED)
		legvelocity[k]=legvelocity[k]*(MAXLEGSPEED/spd);
	}
	// Average velocity of legs
	velocity=(legvelocity[0]+legvelocity[1])/2.0;
    }

    // New position
    position=(legs[0]+legs[1])/2.0;

    // Leftness
    Point legdiff=legs[1]-legs[0];
    leftness=leftness*(1-1/LEFTNESSTC)+legdiff.dot(Point(-velocity.Y(),velocity.X()))/LEFTNESSTC;

    // Age, visibility counters
    if (fs[0].size()==0 && fs[1].size()==0) 
	consecutiveInvisibleCount++;
    else {
	consecutiveInvisibleCount=0;
	totalVisibleCount++;
    }
    age++;
}

void Person::addToMX(mxArray *people, int index) const {
    // const char *fieldnames[]={"id","position","legs","prevlegs","legvelocity","scanpts","posvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount"};
    // Note: for multidimensional arrays, first index changes most rapidly in accessing matlab data
    mxArray *pId = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pId) = id;
    mxSetField(people,index,"id",pId);

    mxArray *pPosition = mxCreateDoubleMatrix(1,2,mxREAL);
    double *data = mxGetPr(pPosition);
    data[0]=position.X()/1000;
    data[1]=position.Y()/1000;
    mxSetField(people,index,"position",pPosition);

    mxArray *pPosvar = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pPosvar);
    data[0]=posvar[0]/1e6;
    data[1]=posvar[1]/1e6;
    mxSetField(people,index,"posvar",pPosvar);

    mxArray *pVelocity = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pVelocity);
    data[0]=velocity.X()/1000;
    data[1]=velocity.Y()/1000;
    mxSetField(people,index,"velocity",pVelocity);

    mxArray *pMinval = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pMinval);
    data[0]=minval.X()/1000;
    data[1]=minval.Y()/1000;
    mxSetField(people,index,"minval",pMinval);

    mxArray *pMaxval = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pMaxval);
    data[0]=maxval.X()/1000;
    data[1]=maxval.Y()/1000;
    mxSetField(people,index,"maxval",pMaxval);

    mxArray *pScanptsCA=mxCreateCellMatrix(1,2);
    for (int i=0;i<2;i++) {
	mxArray *pScanpts = mxCreateDoubleMatrix(scanpts[i].size(),1,mxREAL);
	data = mxGetPr(pScanpts);
	for (unsigned int j=0;j<scanpts[i].size();j++)
	    *data++=scanpts[i][j]+1;		// Need to change 0-based to 1-based
	mxSetCell(pScanptsCA,i,pScanpts);
    }
    mxSetField(people,index,"scanpts",pScanptsCA);

    mxArray *pLegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pLegs);
    *data++=legs[0].X()/1000;
    *data++=legs[1].X()/1000;
    *data++=legs[0].Y()/1000;
    *data++=legs[1].Y()/1000;
    mxSetField(people,index,"legs",pLegs);

    mxArray *pLikeCA=mxCreateCellMatrix(1,2);
    for (int i=0;i<2;i++) {
	mxArray *pLike = mxCreateDoubleMatrix(likeny,likenx,mxREAL);
	assert((int)like[i].size()==likenx*likeny);
	data = mxGetPr(pLike);
	for (unsigned int j=0;j<like[i].size();j++) 
	    *data++=-like[i][j];   // Use neg loglikes in the matlab version
	mxSetCell(pLikeCA,i,pLike);
    }
    mxSetField(people,index,"like",pLikeCA);

    mxArray *pMaxlike = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pMaxlike) = maxlike;
    mxSetField(people,index,"maxlike",pMaxlike);

    mxArray *pPrevlegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pPrevlegs);
    *data++=prevlegs[0].X()/1000;
    *data++=prevlegs[1].X()/1000;
    *data++=prevlegs[0].Y()/1000;
    *data++=prevlegs[1].Y()/1000;
    mxSetField(people,index,"prevlegs",pPrevlegs);

    mxArray *pLegvel = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pLegvel);
    *data++=legvelocity[0].X()/1000;
    *data++=legvelocity[1].X()/1000;
    *data++=legvelocity[0].Y()/1000;
    *data++=legvelocity[1].Y()/1000;
    mxSetField(people,index,"legvelocity",pLegvel);

    mxArray *pLeftness = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pLeftness) = leftness;
    mxSetField(people,index,"leftness",pLeftness);

    mxArray *pLegdiam = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pLegdiam) = legdiam/1000;
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
