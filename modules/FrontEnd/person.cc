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
static const bool USELIKEGRIDFORALL=false;

void People::add(const Point &l1, const Point &l2) {
    // Find a free channel, or advance to next one if none free
    int channel=nextchannel;
    while (size() < NCHANNELS) {
	// Should be able to find a channel
	bool ok=true;
	for (int i=0;i<size();i++) {
	    if (p[i]->getChannel()==channel) {
		// Occupied, advance to next channel
		channel=channel%NCHANNELS+1;
		ok=false;
		break;
	    }
	}
	if (ok)
	    break;
    }
    nextchannel=(channel%NCHANNELS)+1;
    p.push_back(std::shared_ptr<Person>(new Person(nextid,channel,l1,l2)));
    nextid++;
}

Person::Person(int _id, int _channel, const Point &leg1, const Point &leg2) {
    id=_id;
    // Assign legs
    legs[0]=Leg(leg1);
    legs[1]=Leg(leg2);

    trackedBy=1;
    trackedPoints[0]=0;
    trackedPoints[1]=0;
    
    channel=_channel;

    position=(leg1+leg2)/2;
    posvar=(legs[0].posvar+legs[1].posvar)/4;
    age=1;
    consecutiveInvisibleCount=0;
    totalVisibleCount=1;
    dbg("Person",1) << "New person: " << *this << std::endl;
}

Person::~Person() {
    dbg("Person",1) << "Deleting person " << *this << std::endl;
    if (group!=nullptr)
	group->remove(id);
    // NOTE: If we get an assertion error when removing the group, its probably because somewhere in the code a copy of the person is being made;  when that's deleted the dtor is called and it removes the group prematurely
}

bool Person::isDead() const {
    //float visibility = totalVisibleCount*1.0/age;
    bool result = consecutiveInvisibleCount >= ((age<AGETHRESHOLD)?INITIALMAXINVISIBLE:INVISIBLEFORTOOLONG);
    //    bool result = (age<AGETHRESHOLD && visibility<MINVISIBILITY) || (consecutiveInvisibleCount >= INVISIBLEFORTOOLONG);
    if (result) {
	dbg("Person.isDead",2) << " Person has expired life: " << *this << std::endl;
    }
    return result;
}

std::ostream &operator<<(std::ostream &s, const Person &p) {
    s << "ID " << p.id ;
    s << ",ch: " << p.channel;
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
      << ", tvis: " << p.totalVisibleCount
      << std::setprecision(3);
    if (p.consecutiveInvisibleCount > 0)
	s << ",invis:" << p.consecutiveInvisibleCount;
    return s;
}

void Person::predict(int nstep, float fps) {
    if (nstep==0)
	return;

    for (int step=0;step<nstep;step++) {
	for (int i=0;i<2;i++) 
	    legs[i].savePriorPositions();

	for (int i=0;i<2;i++) 
	    legs[i].predict(legs[1-i]);
    }


    position=(legs[0].position+legs[1].position)/2;
    posvar=(legs[0].posvar+legs[1].posvar)/4;
    velocity=(legs[0].velocity+legs[1].velocity)/2;
    dbg("Person.predict",2) << "After predict: " << *this << std::endl;


    // If one leg is locked down, then the other leg can't vary more than MAXLEGSEP
    //    for (int i=0;i<2;i++)
    //	legs[i].posvar=std::min(legs[i].posvar,legs[1-i].posvar+legStats.getSepSigma()*legStats.getSepSigma());
}

// Get likelihood of an observed echo at pt hitting leg given current model
float Person::getObsLike(const Point &pt, int leg, const Vis &vis) const {
    return legs[leg].getObsLike(pt,vis,legStats);
}

// Setup grid for searching for person given that scan points fs[0] and fs[1] are assigned to the two legs
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
	    Point pt=vis.getSick()->getWorldPoint(fs[i][j]);
	    Point lpt=vis.getSick()->getLocalPoint(fs[i][j]);
	    // Compute expected target center for this point
	    Point lphit=lpt+lpt/lpt.norm()*legs[i].getDiam()/2;
	    Point phit=vis.getSick()->localToWorld(lphit);
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
    float maxDiam=std::max(legs[0].getDiam(),legs[1].getDiam());
    minval=minval-maxDiam/2;
    maxval=maxval+maxDiam/2;

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
    dbg("Person.setupGrid",4) << "Search over a " << likenx << " x " << likeny << " grid, diam=" << legs[0].getDiam() << ", " << legs[1].getDiam() << " +/-" << LEGDIAMSIGMA << std::endl;
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
	    double apriorilike=normlike(pt.X(),position.X(),posstd) + normlike(pt.Y(),position.Y(),posstd);
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
    Point oldsepvec=legs[1].getPriorPosition(1)-legs[0].getPriorPosition(1);
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
	    double apriorilike=normlike(newsepvec.X(),oldsepvec.X(),sepstd) + normlike(newsepvec.Y(),oldsepvec.Y(),sepstd);
	    
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
    float minLegSep=std::max(MINLEGSEP,std::max(legStats.getSep()-legStats.getSepSigma(),(legs[0].getDiam()+legs[1].getDiam())/2));
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
    // Only use a single LIDAR for updates at any one time
    // If this is not the LIDAR we were previously using, check if we have more hits and perhaps switch to this one
    if (trackedBy != vis.getSick()->getId()) {
	// Not us, but see how we're doing compared to the current one
	int minpts=std::min(5,trackedPoints[0]+trackedPoints[1]);   // Need to have at least this for each leg to switch
	if ((trackedPoints[0]<2 || trackedPoints[1]<2) && fs[0].size()>=minpts && fs[1].size()>=minpts) {
	    trackedBy=vis.getSick()->getId();
	    dbg("Person.update",1) << "Switching to LIDAR " << trackedBy << " to track person " << id  << "(" << fs[0].size() << ", " << fs[1].size() << " is better than (" << trackedPoints[0] << ", " << trackedPoints[1]  << ") scanpts)" << std::endl;
	} else {
	    // Set the scanPts in the legs though
	    legs[0].setScanPts(fs[0]);
	    legs[1].setScanPts(fs[1]);
	    return;
	}
    }
    trackedPoints[0]=fs[0].size();
    trackedPoints[1]=fs[1].size();
    
    // May eed to run 3 passes, leg0,leg1(which by now includes separation likelihoods),and then leg0 again since it was updated during the 2nd iteration due to separation likelihoods (or swapped of this)
    setupGrid(vis,fs);
    // Process the leg with the most hits first
    int first=0;
    if (fs[0].size()<fs[1].size())
	first=1;
    if (fs[first].size() > 0 || USELIKEGRIDFORALL)
	legs[first].update(vis,bglike,fs[first],legStats,NULL);
    if (fs[1-first].size()>0 || USELIKEGRIDFORALL)  {
	legs[1-first].update(vis,bglike,fs[1-first],legStats,&legs[first]);
	// Probably shouldn't propagate separation from leg with leasst measurements to one with more
	if (fs[first].size() == fs[1-first].size() && false) {
	    // Have the same number of hits, so go ahead an propagate back the leg separation effect
	    dbg("Person.update",2) << "Re-running update of leg[" << first << "] since leg[" << 1-first << "] position changed." << std::endl;
	    legs[first].update(vis,bglike,fs[first],legStats,&legs[1-first]);
	}
    }

    // Combine individual leg likelihoods to make person estimates
    if (USEPERSONLIKE)
	analyzeLikelihoods();

    // Update visibility counters
    legs[0].updateVisibility(bglike);
    legs[1].updateVisibility(bglike);

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
    float minLegSep=std::max(MINLEGSEP,(legs[0].getDiam()+legs[1].getDiam())/2);
    if (legsep<minLegSep) {
	dbg("Person.predict",1) << "legs are " << legsep << " apart (< " << minLegSep << "), moving apart" << std::endl;
	Point vec;
	vec=(legs[0].position-legs[1].position)*(minLegSep/legsep-1);
	legs[0].position=legs[0].position+vec*(legs[0].posvar/(legs[0].posvar+legs[1].posvar));
	legs[1].position=legs[1].position-vec*(legs[1].posvar/(legs[0].posvar+legs[1].posvar));
    }

    dbg("Person.update",2) << "After adjusting leg seps: " << *this << std::endl;

    // Update velocities
    Point l0vel=legs[0].getVelocity();
    legs[0].updateVelocity(nstep,fps,legs[1].getVelocity());
    legs[1].updateVelocity(nstep,fps,l0vel);

    /*
    if (!legs[0].isVisible() && !legs[1].isVisible()) {
	// Both legs hidden, maintain both at average velocity (already damped by legs.update())
	dbg("Person.update",2) << "Person " << id << ": both legs hidden" << std::endl;
	legs[0].velocity=(legs[0].velocity+legs[1].velocity)/2.0;
	legs[1].velocity=legs[0].velocity;
    }
    */
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
    } else  {
	dbg("Person.update",3) << "Both legs invisible -- updating count" << std::endl;
	consecutiveInvisibleCount++;
    }
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
    float avgDiam=(legs[0].getDiam()+legs[1].getDiam())/2;
    if (lo_send(addr, "/pf/update","ififfffffiii",frame,now,id,
		position.X()/UNITSPERM,position.Y()/UNITSPERM,
		velocity.X()/UNITSPERM,velocity.Y()/UNITSPERM,
		(legStats.getSep()+avgDiam)/UNITSPERM,avgDiam/UNITSPERM,
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
		avgDiam/UNITSPERM,(legs[0].getDiamSigma()+legs[1].getDiamSigma())/2/UNITSPERM,
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

#ifdef MATLAB
void Person::addToMX(mxArray *people, int index) const {
    // Fields are defined in world.cc
    // const char *fieldnames[]={"id","position","legs","predictedlegs","prevlegs","legvelocity","scanpts","persposvar", "posvar","prevposvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount","trackedBy"};
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
	if (legs[i].like.size()!=legs[i].likenx*legs[i].likeny) {
	    dbg("Person.addToMX",1) << "legs["<< i << "i].like.size=" << legs[i].like.size() << ", but expected " << legs[i].likenx << "x" << legs[i].likeny << std::endl;
	}
	mxArray *pLike = mxCreateDoubleMatrix(legs[i].likeny,legs[i].likenx,mxREAL);
	data = mxGetPr(pLike);
	for (unsigned int j=0;j<legs[i].like.size();j++) 
	    *data++=-legs[i].like[j];   // Use neg loglikes in the matlab version
	for (unsigned int j=legs[i].like.size();j<legs[i].likenx*legs[i].likeny;j++) 
	    *data++=0;
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
    *data++=legs[0].getPriorPosition(1).X()/UNITSPERM;
    *data++=legs[1].getPriorPosition(1).X()/UNITSPERM;
    *data++=legs[0].getPriorPosition(1).Y()/UNITSPERM;
    *data++=legs[1].getPriorPosition(1).Y()/UNITSPERM;
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
    *mxGetPr(pLegdiam) = legs[0].getDiam()/UNITSPERM;  // FIXME: Send both legs
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

    mxArray *pTrackedBy = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pTrackedBy) = trackedBy;
    mxSetField(people,index,"trackedBy",pTrackedBy);
}
#endif

