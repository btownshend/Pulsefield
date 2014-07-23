#include <vector>
#include <cmath>
#include <string.h>
#include "person.h"
#include "vis.h"
#include "parameters.h"
#include "dbg.h"
#include "normal.h"
#include "lookuptable.h"
#include "groups.h"

static const bool USEPERSONLIKE=false;

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
    posvar=(legs[0].posvar+legs[1].posvar)/4;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;
    dbg("Person",1) << "New person: " << *this << std::endl;
}

Person::~Person() {
    dbg("Person",2) << "Deleting person " << *this << std::endl;
    if (group!=nullptr)
	group->remove(id);
}

bool Person::isDead() const {
    float visibility = totalVisibleCount*1.0/age;
    bool result = (age<AGETHRESHOLD && visibility<MINVISIBILITY) || (consecutiveInvisibleCount >= INVISIBLEFORTOOLONG);
    if (result) {
	dbg("Person.isDead",2) << " Person has expired life: " << *this << std::endl;
    }
    return result;
}

std::ostream &operator<<(std::ostream &s, const Person &p) {
    s << "ID " << p.id ;
    if (p.group != nullptr)
	s << ", GID:" << p.group->getID();
    s << std::fixed << std::setprecision(0) 
      << ", position: " << p.position << "+-" << sqrt(p.posvar)
      << ", leg1:  {" << p.legs[0] << "}"
      << ", leg2: {" << p.legs[1] << "}"
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

    position=(legs[0].position+legs[1].position)/2;
    posvar=(legs[0].posvar+legs[1].posvar)/4;
    velocity=(legs[0].velocity+legs[1].velocity)/2;
    dbg("Person.predict",2) << "After predict: " << *this << std::endl;


    // If one leg is locked down, then the other leg can't vary more than MAXLEGSEP
    //    for (int i=0;i<2;i++)
    //	legs[i].posvar=std::min(legs[i].posvar,legs[1-i].posvar+legStats.getSepSigma()*legStats.getSepSigma());
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Person::getObsLike(const Point &pt, int leg, int frame) const {
    return legs[leg].getObsLike(pt,frame,legStats);
}

void Person::setupGrid(const Vis &vis, const std::vector<int> fs[2]) {
    // Bound search by prior position + 2*sigma(position) + legdiam/2
    float margin;
    margin=2*sqrt(std::max(legs[0].posvar,legs[1].posvar));
    minval=position-margin;
    maxval=position+margin;

    Point legSepVector=legs[1].getPosition()-legs[0].getPosition();

    // Make sure all hits are included
    for (int i=0;i<2;i++)
	for (int j=0;j<fs[i].size();j++)  {
	    Point pt=vis.getSick()->getPoint(fs[i][j]);
	    // Compute expected target center for this point
	    Point phit=pt+pt/pt.norm()*legStats.getDiam()/2;
	    // Adjust point back to person-centered space from leg-centered space
	    if (i==0)
		pt=pt+legSepVector/2;
	    else
		pt=pt-legSepVector/2;
	    minval=minval.min(pt);
	    maxval=maxval.max(pt);
	    minval=minval.min(phit);
	    maxval=maxval.max(phit);
	}

    // Increase search by legdiam/2 to make sure entire PDF is contained
    minval=minval-legStats.getDiam()/2;
    maxval=maxval+legStats.getDiam()/2;

    // Initial estimate of grid size
    float step=10;
    likenx=(int)((maxval.X()-minval.X())/step+1.5);
    likeny=(int)((maxval.Y()-minval.Y())/step+1.5);
    if (likenx*likeny > MAXGRIDPTS) {
	step=step*sqrt(likenx*likeny*1.0/MAXGRIDPTS);
	dbg("Leg.update",3) << "Too many grid points (" << likenx << " x " << likeny << ") - increasing stepsize to  " << step << " mm" << std::endl;
    }

    minval.setX(floor(minval.X()/step)*step);
    minval.setY(floor(minval.Y()/step)*step);
    maxval.setX(ceil(maxval.X()/step)*step);
    maxval.setY(ceil(maxval.Y()/step)*step);

    likenx=(int)((maxval.X()-minval.X())/step+1.5);
    likeny=(int)((maxval.Y()-minval.Y())/step+1.5);
    dbg("Person.setupGrid",4) << "Search box = " << minval << " : " << maxval << std::endl;
    dbg("Person.setupGrid",4) << "Search over a " << likenx << " x " << likeny << " grid, diam=" << legStats.getDiam() << " +/-" << LEGDIAMSIGMA << std::endl;
    dbg("Person.setupGrid",4) << "Legsepvector=" << legSepVector << std::endl;
    legs[0].setupGrid(likenx, likeny, minval-legSepVector/2, maxval-legSepVector/2);
    legs[1].setupGrid(likenx, likeny, minval+legSepVector/2, maxval+legSepVector/2);
}

void Person::analyzeLikelihoods() { 
    // Form likelihood of person center
    // Individual leg likelihoods are already shifted relative to each other by expected leg separation vector, thus can add the same locations in each grid 
    std::vector<float> like1 = legs[0].like;
    std::vector<float> like2=legs[1].like;
    assert(like1.size()==likenx*likeny);
    assert(like2.size()==likenx*likeny);
    like=like1;
    for (int i=0;i<like.size();i++) 
	like[i]+=like2[i];

    float stepx=(maxval.X()-minval.X())/(likenx-1);
    float stepy=(maxval.Y()-minval.Y())/(likeny-1);

    // Apply apriori distribution (bivariate normal with equal variances) to likelikhood function
    double posstd=sqrt(posvar);
    assert(posstd>0);
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*stepx;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*stepy;
	    Point pt(x,y);
	    double apriorilike=log(normpdf(pt.X(),position.X(),posstd)*UNITSPERM) + log(normpdf(pt.Y(),position.Y(),posstd)*UNITSPERM);
	    like[ix*likeny+iy]+=apriorilike;
	}
    }

    // Find iterator that points to maximum of MLE
    std::vector<float>::iterator mle=std::max_element(like.begin(),like.end());
    maxlike=*mle;

    // Use iterator position to figure out location of MLE
    int pos=distance(like.begin(),mle);
    int mleix=pos/likeny;
    int mleiy=pos-mleix*likeny;

    // Set person position to MLE
    Point oldPosition=position;
    position=Point(minval.X()+mleix*stepx,minval.Y()+mleiy*stepy);

    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " MLE position= " << position << " (was  " << oldPosition  << ") with like= " << *mle << std::endl;

    // Calculate variance (actually mean-square distance from MLE)
    double var=0;
    double tprob=0;
    for (int ix=0;ix<likenx;ix++) {
	float x=minval.X()+ix*stepx;
	for (int iy=0;iy<likeny;iy++) {
	    float y=minval.Y()+iy*stepy;
	    Point pt(x,y);
	    if (like[ix*likeny+iy]<-50)
		// Won't add much!
		continue;
	    double prob=exp(like[ix*likeny+iy]);
	    if (std::isnan(prob) || !(prob>0))
		dbg("Person.analyzeLikelihoods",3) << "At ix=" << ix << ", iy=" << iy << "; prob=" << prob << ", like=" << like[ix*likeny+iy] << std::endl;
	    assert(prob>0);
	    var+=prob*pow((pt-position).norm(),2.0);
	    assert(~std::isnan(var));
	    tprob+=prob;
	}
    }
    if (tprob<=0) {
	posvar=pow((maxval-minval).norm(),2.0);
	dbg("Person.analyzeLikelihoods",3) << "Position prob grid totals to " << tprob << "; setting posstd to " << sqrt(posvar) << std::endl;
    } else {
	posvar=var/tprob;
    }
    if (posvar< SENSORSIGMA*SENSORSIGMA/2) {
	dbg("Person.analyzeLikelihoods",3) << "Calculated posvar for leg is too low (" << sqrt(posvar) << "), setting to " << SENSORSIGMA/sqrt(2) << std::endl;
	posvar= SENSORSIGMA*SENSORSIGMA/2;
    }

    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " Position std= " << sqrt(posvar) << std::endl;

    // Now fold over leg likehoods:  overlay leg[1] with leg[0] reflected around person position to find best estimate of leg separation vector
    double sepmaxlike=-1e99;
    int sepmleix,sepmleiy;
    Point sepvec;
    Point oldsepvec=legs[1].prevPosition-legs[0].prevPosition;
    double sepstd=legStats.getSepSigma();
    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " Analyzing leg separation using prior sepvec=" << oldsepvec << " with std=" << sepstd << std::endl;
    seplike.resize(like.size());
    // Figure out ranges for separation grid
    int ix2=2*mleix;
    int iy2=2*mleiy;
    Point p0=legs[0].minval+Point(ix2*stepx,iy2*stepy);
    Point p1=legs[1].minval;
    sepminval=p1-p0;
    sepmaxval=sepminval+(maxval-minval)*2;
    dbg("Person.analyzeLikelihoods",3) << "Separation grid goes from min=" << sepminval << " to max=" << sepmaxval << std::endl;

    for (int ix=0;ix<likenx;ix++) {
	int ix2=2*mleix-ix;
	for (int iy=0;iy<likeny;iy++) {
	    int index1=ix*likeny+iy;
	    if (ix2<0 || ix2>=likenx) {
		seplike[index1]=nan("");
		continue;
	    }
	    int iy2=2*mleiy-iy;
	    if (iy2<0 || iy2>=likeny) {
		seplike[index1]=nan("");
		continue;
	    }
	    Point p0=legs[0].minval+Point(ix2*stepx,iy2*stepy);
	    Point p1=legs[1].minval+Point(ix*stepx,iy*stepy);
	    Point newsepvec=p1-p0;
	    double apriorilike=log(normpdf(newsepvec.X(),oldsepvec.X(),sepstd)*UNITSPERM) + log(normpdf(newsepvec.Y(),oldsepvec.Y(),sepstd)*UNITSPERM);
	    
	    int index2=ix2*likeny+iy2;
	    seplike[index1]=like2[index1]+like1[index2]+apriorilike;
	    if (seplike[index1]>sepmaxlike) {
		sepmaxlike=seplike[index1];
		sepmleix=ix;
		sepmleiy=iy;
		sepvec=newsepvec;
		dbg("Person.analyzeLikelihoods",3) << "At like1(" << ix << "," << iy << ") =" << like1[index1] << ", like2(" << ix2 << "," << iy2 << ")=" << like2[index2] << ", apriori=" << apriorilike << "-> total=" << sepmaxlike << ", sepvec=" << sepvec << std::endl;
	    }
	}
    }
    
    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " leg sep MLE= " << sepvec << " with like=" << sepmaxlike << " at (" << sepmleix << "," << sepmleiy  << ")" << " separate leg positioning gave legvec=" << (legs[1].getPosition()-legs[0].getPosition()) << std::endl;

    // Check that they didn't get too close or too far apart
    float legsep=sepvec.norm();
    float maxLegSep=std::min(MAXLEGSEP,legStats.getSep()+legStats.getSepSigma());
    if (legsep>maxLegSep) {
	dbg("Person.analyzeLikelihoods",2) << "legs are " << legsep << " apart (> " << maxLegSep << "), moving together" << std::endl;
	Point vec;
	sepvec=sepvec*maxLegSep/legsep;
    }
    float minLegSep=std::max(MINLEGSEP,std::max(legStats.getSep()-legStats.getSepSigma(),legStats.getDiam()));
    if (legsep<minLegSep) {
	dbg("Person.predict",2) << "legs are " << legsep << " apart (< " << minLegSep << "), moving apart" << std::endl;
	sepvec=sepvec*minLegSep/legsep;
    }
    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " leg[0] moving from " << legs[0].getPosition() << " to " << position-(sepvec/2) << std::endl;
    dbg("Person.analyzeLikelihoods",3) << "ID " << id << " leg[1] moving from " << legs[1].getPosition() << " to " << position+(sepvec/2) << std::endl;
    // Set the leg positions based on this, but don't set their variance or an incorrect leg position will get locked in
    //legs[0].position=position-(sepvec/2);
    //legs[1].position=position+(sepvec/2); 
}

void Person::update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs[2], int nstep,float fps) {
    // Need to run 3 passes, leg0,leg1(which by now includes separation likelihoods),and then leg0 again since it was updated during the 2nd iteration due to separation likelihoods
    setupGrid(vis,fs);
    legs[0].update(vis,bglike,fs[0],legStats,NULL);
    legs[1].update(vis,bglike,fs[1],legStats,NULL);
    if (false) {
	// TODO only if leg[1] adjusted, then do first leg again
	dbg("Person.update",2) << "Re-running update of leg[0] since leg[1] position changed." << std::endl;
	legs[0].update(vis,bglike,fs[0],legStats,&legs[1]);
    }

    // Combine individual leg likelihoods to make person estimates
    if (USEPERSONLIKE)
	analyzeLikelihoods();

    // Update visibility counters
    legs[0].updateVisibility();
    legs[1].updateVisibility();

    // Update leg diameter estimates in legStats
    legs[0].updateDiameterEstimates(vis,legStats);
    legs[1].updateDiameterEstimates(vis,legStats);

    // Other stats to update
    legStats.update(*this);
 
    // Check that they didn't get too close or too far apart
    float legsep=(legs[0].position-legs[1].position).norm();
    float maxLegSep=MAXLEGSEP;
    if (legsep>maxLegSep) {
	dbg("Person.update",2) << "legs are " << legsep << " apart (> " << maxLegSep << "), moving together" << std::endl;
	Point vec;
	vec=(legs[0].position-legs[1].position)*(maxLegSep/legsep-1);
	legs[0].position=legs[0].position+vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
	legs[1].position=legs[1].position-vec*(legs[1].posvar/(legs[0].posvar+legs[1].posvar));
    }
    float minLegSep=std::max(MINLEGSEP,legStats.getDiam());
    if (legsep<minLegSep) {
	dbg("Person.predict",2) << "legs are " << legsep << " apart (< " << minLegSep << "), moving apart" << std::endl;
	Point vec;
	vec=(legs[0].position-legs[1].position)*(minLegSep/legsep-1);
	legs[0].position=legs[0].position+vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
	legs[1].position=legs[1].position-vec*(legs[1].posvar/(legs[0].posvar+legs[1].posvar));
    }

    dbg("Person.predict",2) << "After adjusting leg seps: " << *this << std::endl;

    // Update velocities
    legs[0].updateVelocity(nstep,fps);
    legs[1].updateVelocity(nstep,fps);

    if (!legs[0].isVisible() && !legs[1].isVisible()) {
	// Both legs hidden, maintain both at average velocity (already damped by legs.update())
	dbg("Person.update",2) << "Person " << id << ": both legs hidden" << std::endl;
	legs[0].velocity=(legs[0].velocity+legs[1].velocity)/2.0;
	legs[1].velocity=legs[0].velocity;
    }

    if (!USEPERSONLIKE) {
	// Average velocity of legs
	velocity=(legs[0].velocity+legs[1].velocity)/2.0;
	assert(std::isfinite(velocity.X()) && std::isfinite(velocity.Y()));

	// New position
	position=(legs[0].position+legs[1].position)/2.0;
    } 

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
    int groupid=0; 
    int groupsize=1;
    if (group) {
	groupid = group->getID();
	groupsize = group->size();
    }
    if (lo_send(addr, "/pf/update","ififfffffiii",frame,now,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		velocity.X()/UNITSPERM,velocity.Y()/UNITSPERM,
		(legStats.getSep()+legStats.getDiam())/UNITSPERM,legStats.getDiam()/UNITSPERM,
		groupid,groupsize,
		channel) < 0) {
	std::cerr << "Failed send of /pf/update to " << lo_address_get_url(addr) << std::endl;
	return;
    }
    if (lo_send(addr, "/pf/body","iifffffffffffffffi",frame,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		sqrt(posvar)/UNITSPERM,sqrt(posvar)/UNITSPERM,
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

void Person::addToGroup(std::shared_ptr<Group> g) {
    assert(group==nullptr);
    group=g; group->add(id);
}

void Person::unGroup() {
    assert(group!=nullptr);
    group->remove(id);
    group.reset();
}

void Person::addToMX(mxArray *people, int index) const {
    // const char *fieldnames[]={"id","position","legs","predictedlegs","prevlegs","legvelocity","scanpts","persposvar", "posvar","prevposvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount"};
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
    data[0]=legs[0].posvar/(UNITSPERM*UNITSPERM);
    data[1]=legs[1].posvar/(UNITSPERM*UNITSPERM);
    mxSetField(people,index,"posvar",pPosvar);

    mxArray *pPPosvar = mxCreateDoubleMatrix(1,1,mxREAL);
    data = mxGetPr(pPPosvar);
    data[0]=posvar/(UNITSPERM*UNITSPERM);
    mxSetField(people,index,"persposvar",pPPosvar);

    mxArray *pPrevposvar = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pPrevposvar);
    data[0]=legs[0].prevposvar/(UNITSPERM*UNITSPERM);
    data[1]=legs[1].prevposvar/(UNITSPERM*UNITSPERM);
    mxSetField(people,index,"prevposvar",pPrevposvar);

    mxArray *pVelocity = mxCreateDoubleMatrix(1,2,mxREAL);
    data = mxGetPr(pVelocity);
    data[0]=velocity.X()/UNITSPERM;
    data[1]=velocity.Y()/UNITSPERM;
    mxSetField(people,index,"velocity",pVelocity);

    mxArray *pMinval = mxCreateDoubleMatrix(4,2,mxREAL);
    data = mxGetPr(pMinval);
    for (int i=0;i<2;i++) 
	*data++=legs[i].minval.X()/UNITSPERM;
    *data++=minval.X()/UNITSPERM;
    *data++=sepminval.X()/UNITSPERM;
    for (int i=0;i<2;i++) 
	*data++=legs[i].minval.Y()/UNITSPERM;
    *data++=minval.Y()/UNITSPERM;
    *data++=sepminval.Y()/UNITSPERM;
    mxSetField(people,index,"minval",pMinval);

    mxArray *pMaxval = mxCreateDoubleMatrix(4,2,mxREAL);
    data = mxGetPr(pMaxval);
    for (int i=0;i<2;i++) 
	*data++=legs[i].maxval.X()/UNITSPERM;
    *data++=maxval.X()/UNITSPERM;
    *data++=sepmaxval.X()/UNITSPERM;
    for (int i=0;i<2;i++) 
	*data++=legs[i].maxval.Y()/UNITSPERM;
    *data++=maxval.Y()/UNITSPERM;
    *data++=sepmaxval.Y()/UNITSPERM;
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

    mxArray *pPredictedLegs = mxCreateDoubleMatrix(2,2,mxREAL);
    data = mxGetPr(pPredictedLegs);
    *data++=legs[0].predictedPosition.X()/UNITSPERM;
    *data++=legs[1].predictedPosition.X()/UNITSPERM;
    *data++=legs[0].predictedPosition.Y()/UNITSPERM;
    *data++=legs[1].predictedPosition.Y()/UNITSPERM;
    mxSetField(people,index,"predictedlegs",pPredictedLegs);


    mxArray *pLikeCA=mxCreateCellMatrix(1,USEPERSONLIKE?4:2);
    for (int i=0;i<2;i++) {
	mxArray *pLike = mxCreateDoubleMatrix(legs[i].likeny,legs[i].likenx,mxREAL);
	assert((int)legs[i].like.size()==legs[i].likenx*legs[i].likeny);
	data = mxGetPr(pLike);
	for (unsigned int j=0;j<legs[i].like.size();j++) 
	    *data++=-legs[i].like[j];   // Use neg loglikes in the matlab version
	mxSetCell(pLikeCA,i,pLike);
    }
    
    if (USEPERSONLIKE) {
	// Person likelihood
	mxArray *pPLike = mxCreateDoubleMatrix(likeny,likenx,mxREAL);
	assert((int)like.size()==likenx*likeny);
	data = mxGetPr(pPLike);
	for (unsigned int j=0;j<like.size();j++) 
	    *data++=-like[j];   // Use neg loglikes in the matlab version
	mxSetCell(pLikeCA,2,pPLike);

	// Sep likelihood
	mxArray *pSeplike = mxCreateDoubleMatrix(likeny,likenx,mxREAL);
	assert((int)seplike.size()==likenx*likeny);
	data = mxGetPr(pSeplike);
	for (unsigned int j=0;j<seplike.size();j++) 
	    *data++=-seplike[j];   // Use neg loglikes in the matlab version
	mxSetCell(pLikeCA,3,pSeplike);
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

