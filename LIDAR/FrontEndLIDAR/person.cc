#include <vector>

#include <string.h>
#include "person.h"
#include "vis.h"
#include "parameters.h"
#include "dbg.h"
#include "normal.h"
#include "lookuptable.h"


Person::Person(int _id, const Point &leg1, const Point &leg2) {
    id=_id;
    // Assign legs
    legs[0]=Leg(leg1);
    legs[1]=Leg(leg2);

    // Find a free channel, or advance to next one if none free
    static int lastchannel=0;
    channel=lastchannel+1;
    lastchannel = (lastchannel+1)%NCHANNELS;

    position=(leg1+leg2)/2;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;

    groupid=-1;
    groupsize=1;
}

Person::~Person() {
    if (groupid!=-1)
	std::cerr << "Warning: deleting person " << id << " who is still a member of group " << groupid << std::endl;
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
      << ", GID:" << p.groupid
      << std::fixed << std::setprecision(0) 
      << ", position: " << p.position
      << ", leg1: " << p.legs[0]
      << ", leg2: " << p.legs[1]
      << ", " << p.legStats
      << std::setprecision(2)
      << ", vel: " << p.velocity
      << ", age: " << p.age
      << std::setprecision(3);
    if (p.consecutiveInvisibleCount > 0)
	s << ",invis:" << p.consecutiveInvisibleCount;
    return s;
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

// Get likelihood of an observed echo at pt hitting leg given current model
float Person::getObsLike(const Point &pt, int leg, int frame) const {
    return legs[leg].getObsLike(pt,frame,legStats);
}

void Person::update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs[2], int nstep,float fps) {
    // Need to run 3 passes, leg0,leg1(which by now includes separation likelihoods),and then leg0 again since it was updated during the 2nd iteration due to separation likelihoods
    legs[0].update(vis,bglike,fs[0],nstep,fps,legStats,0);
    legs[1].update(vis,bglike,fs[1],nstep,fps,legStats,&legs[0]);
    if (true) {
	// TODO only if leg[1] adjusted, then do first leg again
	dbg("Person.update",2) << "Re-running update of leg[0] since leg[1] position changed." << std::endl;
	legs[0].update(vis,bglike,fs[0],nstep,fps,legStats,&legs[1]);
    }
    // Update visibility counters
    legs[0].updateVisibility();
    legs[1].updateVisibility();

    // Update leg diameter estimates in legStats
    legs[0].updateDiameterEstimates(vis,legStats);
    legs[1].updateDiameterEstimates(vis,legStats);

    if (~legs[0].isVisible() && ~legs[1].isVisible()) {
	// Both legs hidden, maintain both at average velocity (already damped by legs.updat())
	legs[0].velocity=(legs[0].velocity+legs[1].velocity)/2.0;
	legs[1].velocity=legs[0].velocity;
    }

    // Average velocity of legs
    velocity=(legs[0].velocity+legs[1].velocity)/2.0;
    assert(isfinite(velocity.X()) && isfinite(velocity.Y()));

    // New position
    position=(legs[0].position+legs[1].position)/2.0;

    // Other stats to update
    legStats.update(*this);
 
    // Age, visibility counters
    if (legs[0].isVisible() || legs[1].isVisible()) {
	consecutiveInvisibleCount=0;
	totalVisibleCount++;
    } else 
	consecutiveInvisibleCount++;
    age++;

    dbg("Person.update",2) << "Done: " << *this << std::endl;
}

// Send /pf/ OSC messages
void Person::sendMessages(lo_address &addr, int frame, double now) const {
    if (lo_send(addr, "/pf/update","ififfffffiii",frame,now,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		velocity.X()/UNITSPERM,velocity.Y()/UNITSPERM,
		(legStats.getSep()+legStats.getDiam())/UNITSPERM,legStats.getDiam()/UNITSPERM,
		groupid,groupsize,
		channel) < 0) {
	std::cerr << "Failed send of /pf/update to " << lo_address_get_url(addr) << std::endl;
	return;
    }
    float posvar=sqrt((legs[0].posvar+legs[1].posvar)/2);
    if (lo_send(addr, "/pf/body","iifffffffffffffffi",frame,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		posvar/UNITSPERM,posvar/UNITSPERM,
		velocity.norm()/UNITSPERM,0.0f,
		velocity.getTheta()*180.0/M_PI,0.0f,
		legStats.getFacing()*180.0/M_PI,legStats.getFacingSEM()*180.0/M_PI,
		legStats.getDiam()/UNITSPERM,legStats.getDiamSigma()/UNITSPERM,
		legStats.getSep()/UNITSPERM,legStats.getSepSigma()/UNITSPERM,
		legStats.getLeftness(),
		consecutiveInvisibleCount) < 0)
	std::cerr << "Failed send of /pf/body to " << lo_address_get_url(addr) << std::endl;
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
