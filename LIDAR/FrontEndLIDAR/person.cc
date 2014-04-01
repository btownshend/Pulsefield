#include <vector>
#include <string.h>
#include <algorithm>
#include "person.h"
#include "vis.h"
#include "parameters.h"
#include "likelihood.h"

Person::Person(int _id, const Vis &vis, const Target *target1, const Target *target2) {
    id=_id;
    legdiam=INITLEGDIAM;
    legs[0]=circmodel(target1,false);
    legclasses[0]=target1->getClass();
    if (target2!=NULL) {
	legs[1]=circmodel(target2,false);
	legclasses[1]=target2->getClass();
    } else {
	legs[1].setThetaRange(legs[0].getTheta(),legs[0].getRange()+legdiam);
	legclasses[1]=1;
    }
    prevlegs[0]=legs[0];
    prevlegs[1]=legs[1];
    position=Point((legs[0].X()+legs[1].X())/2.0,(legs[0].Y()+legs[1].Y())/2.0);
    if (target1==NULL)
	posvar[0]=INITIALHIDDENVAR;
    else
	posvar[0]=INITIALPOSITIONVAR;
    if (target2==NULL)
	posvar[1]=INITIALHIDDENVAR;
    else
	posvar[1]=INITIALPOSITIONVAR;
    leftness=0.0;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;
}

bool Person::isDead() const {
    float visibility = totalVisibleCount*1.0/age;
    bool result = (age<AGETHRESHOLD && visibility<MINVISIBILITY) || (consecutiveInvisibleCount >= INVISIBLEFORTOOLONG);
    if (result) {
	printf(" Deleting ");
	print();
    }
    return result;
}

void Person::print() const {
    printf(" ID:%d, position: (%.0f,%.0f) legs:(%.0f,%.0f)+-%.0f, (%.0f, %.0f)+-%.0f  vel: (%f,%f) age: %d\n",
	   id, position.X(), position.Y(),
	   legs[0].X(), legs[0].Y(), sqrt(posvar[0]),
	   legs[1].X(), legs[1].Y(), sqrt(posvar[1]), 
	   velocity.X(), velocity.Y(),
	   age
	   );
}

void Person::predict(int nstep, float fps) {
    for (int i=0;i<2;i++) {
	prevlegs[i]=legs[i];
	legs[i].setX(legs[i].X()+legvelocity[i].X()*nstep/fps);
	legs[i].setY(legs[i].Y()+legvelocity[i].Y()*nstep/fps);
	posvar[i]+=DRIFTVAR*nstep;
	posvar[i]=std::min(posvar[i],posvar[1-i]+MAXLEGSEP*MAXLEGSEP);
    }
    // Check that they didn't get too close or too far apart
    float legsep=(legs[0]-legs[1]).norm();
    if (legsep<legdiam) {
	printf("predict: legs are %.0f apart (< %.0f), splitting\n", legsep, legdiam);
	Point vec;
	if (legsep>0)
	    vec=(legs[0]-legs[1])*(legdiam/legsep-1);
	else
	    vec=Point(legdiam,0);

	legs[0]=legs[0]+vec*(posvar[0]/(posvar[0]+posvar[1]));
	legs[1]=legs[1]-vec*(posvar[1]/(posvar[0]+posvar[1]));
    }
    if (legsep>MAXLEGSEP) {
	printf("predict: legs are %.0f apart (> %.0f), moving together\n", legsep, MAXLEGSEP);
	Point vec;
	vec=(legs[0]-legs[1])*(MAXLEGSEP/legsep-1);
	legs[0]=legs[0]+vec*(posvar[0]/(posvar[0]+posvar[1]));
	legs[1]=legs[1]-vec*(posvar[1]/(posvar[0]+posvar[1]));
    }
    position.setX((legs[0].X()+legs[1].X())/2.0);
    position.setY((legs[0].Y()+legs[1].Y())/2.0);
    velocity.setX((legvelocity[0].X()+legvelocity[1].X())/2.0);
    velocity.setY((legvelocity[0].Y()+legvelocity[1].Y())/2.0);
}

// Get log like of given x for a zero-mean gaussian with given variance
float normloglike(float var, float x) {
    float sigma=sqrt(var);
    float z=x/sigma;
    return -0.5*z*z-log(sqrt(2*M_PI)*sigma/1000);
}

void Person::getclasslike(const Targets &targets, const Vis &vis, Likelihood &likes, int tracknum) {
    // Compute like of each class assignment from vis to the 2 legs
    // Class 1 represents leg not being visible (shadowed)
    // Check assignments
    float penalties=0;
    if (age<AGETHRESHOLD)
	penalties+=YOUNGPENALTY;

    for (unsigned int i=0;i<targets.size();i++) {
	const Target *t1=&targets[i];
	Point cp1=t1->getCenter();
	if ((legs[0]-cp1).norm() < MAXMOVEMENT+sqrt(posvar[0])) {
	    // Possible leg assignment
	    Point newleg0=circmodel(t1, false);
	    float l0=normloglike(posvar[0],(legs[0]-newleg0).norm());
	    for (unsigned int j=0;j<targets.size();j++) {
		if (i==j)
		    continue;
		const Target *t2=&targets[j];
		Point cp2=t2->getCenter();
		if ((legs[1] -cp2).norm() < MAXMOVEMENT) {
		    Point newleg1=circmodel(t2,false);
		    float l1=normloglike(posvar[1],(legs[1]-newleg1).norm());
		    likes.add(Assignment(tracknum,t1,t2,l0+l1-penalties));
		}
	    }
	    // Leg2 hidden
	    Point hidden1=nearestShadowed(vis,newleg0,legs[1]);
	    float l1=normloglike(posvar[1],(legs[1]-hidden1).norm());
	    likes.add(Assignment(tracknum,t1,NULL,l0+l1-HIDDENPENALTY-penalties));
	}
	if ((legs[1] -cp1).norm() < MAXMOVEMENT+sqrt(posvar[1])) {
	    Point newleg1=circmodel(t1, false);
	    float l1=normloglike(posvar[1],(legs[1]-newleg1).norm());
	    // Leg1 hidden
	    Point hidden0=nearestShadowed(vis,newleg1,legs[0]);
	    float l0=normloglike(posvar[0],(legs[0]-hidden0).norm());
	    likes.add(Assignment(tracknum,NULL,t1,l0+l1-HIDDENPENALTY-penalties));
	}
    }
    // Both legs hidden
    Point hidden0=nearestShadowed(vis,legs[1],legs[0]);
    Point hidden1=nearestShadowed(vis,hidden0,legs[1]);
    float l0=normloglike(posvar[0],(legs[0]-hidden0).norm());
    float l1=normloglike(posvar[1],(legs[1]-hidden1).norm());
    likes.add(Assignment(tracknum,NULL,NULL,l0+l1-2*HIDDENPENALTY-penalties));
}

void Person::newclasslike(const Targets &targets, const Vis &vis, Likelihood &likes)  {
    for (unsigned int i=0;i<targets.size();i++) {
	const Target *t1=&targets[i];
	Point cp1=t1->getCenter();
	for (unsigned int j=0;j<targets.size();j++) {
	    if (i==j)
		continue;
	    const Target *t2=&targets[j];
	    Point cp2=t2->getCenter();
	    if ( (cp2-cp1).norm() <= MAXLEGSEP )
		likes.add(Assignment(-1,t1, t2, normloglike(INITIALPOSITIONVAR,NEWTRACKEQUIVDIST2)));
	}
	// Make a single leg assignment less likely
	likes.add(Assignment(-1,t1, NULL, normloglike(INITIALPOSITIONVAR,NEWTRACKEQUIVDIST1)));
    }
}

void Person::update(const Vis &vis, const Target *t1, const Target *t2, int nstep,float fps) {
    if (t1!=NULL) {
	legs[0]=circmodel(t1,true);
	posvar[0]=INITIALPOSITIONVAR;
	legclasses[0]=t1->getClass();
    } else
	legclasses[0]=1;
    if (t2!=NULL) {
	legs[1]=circmodel(t2,true);
	posvar[1]=INITIALPOSITIONVAR;
	legclasses[1]=t2->getClass();
    } else
	legclasses[1]=1;
    if (t1==NULL)
	legs[0]=nearestShadowed(vis,legs[1],legs[0]);
    if (t2==NULL)
	legs[1]=nearestShadowed(vis,legs[0],legs[1]);

    // Update velocity
    if (t1==NULL && t2==NULL) {
	// Both legs hidden, maintain both at average velocity (damped)
	legvelocity[0]=(legvelocity[0]+legvelocity[1])/2.0*VELDAMPING;
	legvelocity[1]=legvelocity[0];
    } else {
	if (t1==NULL && consecutiveInvisibleCount>0)
	    legvelocity[0]=legvelocity[0]*VELDAMPING;
	else
	    legvelocity[0]=legvelocity[0]*(1-1/VELUPDATETC)+(legs[0]-prevlegs[0])/(nstep/fps)/VELUPDATETC;
	if (t2==NULL && consecutiveInvisibleCount>0)
	    legvelocity[1]=legvelocity[1]*VELDAMPING;
	else
	    legvelocity[1]=legvelocity[1]*(1-1/VELUPDATETC)+(legs[1]-prevlegs[1])/(nstep/fps)/VELUPDATETC;
    }
    // Reduce speed if over maximum
    for (int k=0;k<2;k++) {
	float spd=legvelocity[k].norm();
	if (spd>MAXLEGSPEED)
	    legvelocity[k]=legvelocity[k]*(MAXLEGSPEED/spd);
    }
    // Average velocity of legs
    velocity=(legvelocity[0]+legvelocity[1])/2.0;

    Point newpos=(legs[0]+legs[1])/2.0;
    Point delta=newpos-position;
    Point legdiff=legs[1]-legs[0];
    leftness=leftness*(1-1/LEFTNESSTC)+legdiff.dot(Point(-delta.Y(),delta.X()))/LEFTNESSTC;

    position=newpos;
    if (t1==NULL && t2==NULL) 
	consecutiveInvisibleCount++;
    else {
	consecutiveInvisibleCount=0;
	totalVisibleCount++;
    }
    age++;
}

Point Person::circmodel(const Target *t,  bool update) {
    const std::vector<Point> &pts = t->getPoints();
    // Calculate range to target center
    float range;
    if (pts.size()==1) {
        // Just one point 
        // Sometimes scan will average two nearby targets, thus making the mostly shadowed edge appear too close
        range=pts[0].getRange();
	float prange=t->getPriorPoint().getRange();
	float nrange=t->getNextPoint().getRange();
        if (prange>0 && prange<range)
	    range=std::max(range+legdiam/4,prange+legdiam*5/4);
	else if (nrange>0 && nrange<range)
	    range=std::max(range+legdiam/4,nrange+legdiam*5/4);
	else
	    range+=legdiam/4;
    } else {
	// Set range to average distance + legdiam/4
	range=pts[0].getRange();
	for (unsigned int i=1;i<pts.size();i++)
	    range+=pts[i].getRange();
	range/=pts.size();
	range+=legdiam/4;
    }
    // Calculate angle to target center
    const float anglewidth=legdiam/range;
    const float maxangle=pts.back().getTheta();
    const float minangle=pts[0].getTheta();
    float angle;
    if (t->isLeftShadowed() && ~t->isRightShadowed()) 
	angle=maxangle-anglewidth/2;
    else if (~t->isLeftShadowed() && t->isRightShadowed()) 
	angle=minangle+anglewidth/2;
    else if (t->isLeftShadowed() && t->isRightShadowed()) 
	angle=(minangle+maxangle)/2;
    else {
	// Unshadowed
	float r=range*(maxangle-minangle);
	if (update)
	    legdiam=legdiam*(1-1.0/LEGDIAMTC)+r/LEGDIAMTC;
	angle=(minangle+maxangle)/2;
    }
    Point result;
    result.setThetaRange(angle,range);
    //printf("circmodel(%d)=(%.0f,%.0f)\n", t->getClass(), result.X(), result.Y());
    return result;
}

// Find a point in shadow that could be hiding a leg as close as possible to targetpos
// Targetpos may be corrected to be in the range [legdiam,MAXLEGSEP] from otherlegpos
Point Person::nearestShadowed(const Vis &vis,Point otherlegpos,Point targetpos) {
    Point pos=targetpos;
    float legsep=(otherlegpos-pos).norm();
    if (legsep==0) {
	fprintf(stderr,"Both legs at same position in frame %d - splitting\n", vis.getSick()->getFrame());
	targetpos.setX(targetpos.X()+legdiam);
	legsep=legdiam;
    }
    if (legsep > MAXLEGSEP) {
	// Too far away
	Point dir=(pos-otherlegpos)/legsep;
	pos=otherlegpos+(dir*MAXLEGSEP);
    } else if  (legsep < legdiam) {
	Point dir=(pos-otherlegpos)/legsep;
	pos=otherlegpos+(dir*legdiam);
    }
    float theta=pos.getTheta();
    float range=pos.getRange();
    const float anglewidth=legdiam/range*HIDDENLEGSCALING;   // Make it a little smaller to allow it to be hidden
    if (theta+anglewidth/2 <= vis.getSick()->getAngle(0) || theta-anglewidth/2 >= vis.getSick()->getAngle(vis.getSick()->getNumMeasurements()-1)) {
	// Already outside of FOV
	return pos;
    }
    int cpos1=vis.getSick()->getScanAtAngle((theta-anglewidth/2)*180/M_PI);
    int cpos2=vis.getSick()->getScanAtAngle((theta+anglewidth/2)*180/M_PI);
    bool hidden=true;
    for (int i=cpos1;i<=cpos2;i++) {
	if (vis.getSick()->getRange(0)[i] >= range) {
	    hidden=false;
	    break;
	}
    }
    if (hidden) {
	// Already hidden
	return pos;
    }

    // Current prediction is not shadowed
    // Find shadows that can hide this object
    // ledge,redge,srange gives left, right indices, distance of each shadow
    float bestdist=1e10;
    Point bestpos;
    int besti,bestj;
    float res=vis.getSick()->getScanRes()*M_PI/180;
    for (unsigned int i=0;i<vis.getSick()->getNumMeasurements();i++) {
	float minrange=vis.getSick()->getRange(0)[i];
	for (unsigned int j=i+1;j<vis.getSick()->getNumMeasurements() && j<=i+(cpos2-cpos1);j++) {
	    minrange=std::max(minrange,(float)vis.getSick()->getRange(0)[j]);
	    float shadowrange=std::max((float)minrange,legdiam*HIDDENLEGSCALING/((j-i)*res));
	    if (i==0 || j==vis.getSick()->getNumMeasurements()) {
		// End scans -- object can be outside FOV partially
		shadowrange=minrange;
	    }
	    shadowrange=std::max(range,shadowrange);   // TODO: this is slightly suboptimal
	    shadowrange=std::min(shadowrange,(float)MAXRANGE);   // anything outside is invisible
	    Point shadowpos;
	    shadowpos.setThetaRange((vis.getSick()->getAngle(i)+vis.getSick()->getAngle(j))/2*M_PI/180, shadowrange);
	    float shadowdist=(shadowpos-pos).norm();
	    if (shadowdist<bestdist) {
		bestdist=shadowdist;
		bestpos=shadowpos;
		besti=i;
		bestj=j;
	    }
	    if (shadowrange==range)
		// Already wide enough to get best position, no sense going wider
		break;
	}
    }
    //float distmoved=(bestpos-targetpos).norm();
    //printf("Best location for (%.2f,%.2f) is in shadow %d-%d at (%.2f,%.2f) with a distance of %.2f\n", targetpos.X(), targetpos.Y(), besti, bestj, bestpos.X(), bestpos.Y(), distmoved);
    return bestpos;
}

void Person::addToMX(mxArray *people, int index) const {
    // const char *fieldnames[]={"id","position","legs","prevlegs","legvelocity","legclasses","posvar","velocity","legdiam","leftness","age","consecutiveInvisibleCount","totalVisibleCount"};
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

    mxArray *pLegclasses = mxCreateNumericMatrix(1,2,mxUINT32_CLASS,mxREAL);
    int *idata = (int *)mxGetPr(pLegclasses);
    idata[0]=legclasses[0];
    idata[1]=legclasses[1];
    mxSetField(people,index,"legclasses",pLegclasses);

    mxArray *pLegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pLegs);
    *data++=legs[0].X()/1000;
    *data++=legs[1].X()/1000;
    *data++=legs[0].Y()/1000;
    *data++=legs[1].Y()/1000;
    mxSetField(people,index,"legs",pLegs);

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
