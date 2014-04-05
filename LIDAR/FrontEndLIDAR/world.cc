#include "lo/lo.h"
#include "world.h"
#include "likelihood.h"
#include "vis.h"
#include "dbg.h"
#include "parameters.h"

World::World() {
    lastframe=0;
    nextid=1;
    starttime.tv_sec=0;
    starttime.tv_usec=0;
    initWindow();
}

void World::makeAssignments(const Vis &vis, float entrylike) {
    // Calculate likelihoods of each scan belonging to each track and assign to highest likelihood
    char dbgstr[100];
    sprintf(dbgstr,"Frame.%d",vis.getSick()->getFrame());

    const SickIO *sick=vis.getSick();
    bestlike.resize(sick->getNumMeasurements());
    assignments.resize(sick->getNumMeasurements());
    legassigned.resize(sick->getNumMeasurements());

    const Background *bg=vis.getClassifier()->getBackground();
    const std::vector<float> &bgprob=bg->isbg(*sick);

    for (unsigned int f=0;f<sick->getNumMeasurements();f++) {
        bestlike[f]=entrylike;
	assignments[f]=-2;
	legassigned[f]=0;
	if (bgprob[f]>0) {
	    float bglike=log(bgprob[f]);
	    if (bglike > bestlike[f]) {
		bestlike[f]=bglike;
		assignments[f]=-1;
	    }
	}
	for (unsigned int i=0;i<people.size();i++) {
	    for (int leg=0;leg<2;leg++) {
		// This is a fudge since the DIAMETER is log-normal and the position itself is normal
		float like =people[i].getObsLike(sick->getPoint(f),leg,vis.getSick()->getFrame());
		dbg(dbgstr,20) << "For assigning scan " << f << " to P" << people[i].getID() << "." << leg << ", like=" << like << ", best so far=" << bestlike[f] << ", bgprob=" << bgprob[f] << ", entrylike=" << entrylike << std::endl;
		if (like>bestlike[f]) {
		    bestlike[f]=like;
		    assignments[f]=i;
		    legassigned[f]=leg;
		}
	    }
	}
	if (assignments[f]>=0)
	    dbg("World.makeAssignments",2) << "Assigned scan " << f << " to track " << assignments[f] << "." << legassigned[f] << std::endl;
    }
}

void World::track( const Vis &vis, int frame, float fps) {
    int nsteps;
    if (lastframe>0)
	nsteps=frame-lastframe;
    else
	nsteps=1;
    lastframe=frame;

    // Update existing tracks with next prediction
    for (unsigned int i=0;i<people.size();i++)
	people[i].predict(nsteps,fps);

    int num_measurements=vis.getSick()->getNumMeasurements();
    float entryprob=1-exp(-ENTRYRATE/60.0*nsteps/fps);
    float entrylike=log(entryprob/num_measurements*5);  // Like that a scan is a new entry (assumes 5 hits on avg)
    dbg("World.track",2) << "Tracking frame " << frame << ":  entrylike=" <<  entrylike << std::endl;

    // Map scans to tracks
    makeAssignments(vis,entrylike);
    std::vector<int> unassigned;
    for (unsigned int i=0;i<assignments.size();i++)
	if (assignments[i]==-2)
	    unassigned.push_back(i);

    if (unassigned.size()>0) {
	dbg("World.track",2) << "Have " << unassigned.size() << " scan points unassigned." << std::endl;
	if ((int)unassigned.size()<MINCREATEHITS) {
	    dbg("World.track",3) <<  "'Need at least " << MINCREATEHITS << " points to assign a new track -- skipping" << std::endl;
	} else {
	    // Create a new track if we can find 2 points that are separated by ~meanlegsep
	    float bestsep=1e10;
	    int bestindices[2];
	    for (std::vector<int>::iterator i=unassigned.begin();i!=unassigned.end();i++) {
		for (std::vector<int>::iterator j=unassigned.begin();j!=unassigned.end();j++) {
		    if (i==j)
			continue;
		    float dist=(vis.getSick()->getPoint(*i)-vis.getSick()->getPoint(*j)).norm();
		    if (fabs(dist-MEANLEGSEP)<fabs(bestsep-MEANLEGSEP) ) {
			bestsep=dist;
			dbg("World.track",4) << "Unassigned points " << *i << " and " << *j << " are best separation so far at " << bestsep << std::endl;
			bestindices[0]=*i;
			bestindices[1]=*j;
		    }
		}
	    }
	    if (fabs(bestsep-MEANLEGSEP)<LEGSEPSTD*2 ) {
		dbg("World.track",1) << "Creating an initial track using scans " << bestindices[0] << "," << bestindices[1] << " with separation " << bestsep << std::endl;
		Point l1=vis.getSick()->getPoint(bestindices[0]);
		Point l2=vis.getSick()->getPoint(bestindices[1]);
		people.push_back(Person(nextid,l1,l2));
		nextid++;
		makeAssignments(vis,entrylike);// Redo after adding new tracks, but only do twice (allowing only 1 new person per frame) to limit cpu
	    } else {
		dbg("World.track",2) << "Not creating a track - no pair appropriately spaced: best separation=" << bestsep << std::endl;
	    }
	}
    }

    // Implement assignment
    for (unsigned int i=0;i<people.size();i++) {
	// Build list of points assigned to this person
	std::vector<int> fs[2];
	for (unsigned int j=0;j<assignments.size();j++)
	    if (assignments[j]==(int)i)
		fs[legassigned[j]].push_back(j);
	people[i].update(vis,fs,nsteps,fps);
    }

    // Delete lost people
    for (unsigned int i=0;i<people.size();i++)
	if (people[i].isDead()) {
	    people.erase(people.begin()+i);
	    i--;
	}

    if (DebugCheck("World.track",2) && people.size() > 0) {
	dbg("World.track",2)  << "People at end of frame " <<  lastframe << ":" << std::endl;
	for (unsigned int i=0;i<people.size();i++)
	    dbg("World.track",2)  << people[i] << std::endl;
    }
    if (frame%1==0)
	draw();
}
        
void World::sendMessages(const Destinations &dests, const struct timeval &acquired) {
    dbg("World.sendMessages",5) << "ndest=" << dests.count() << std::endl;
    bool sendstart=false;
    if  (starttime.tv_sec==0) {
	starttime=acquired;
	sendstart=true;
    }
    double now=(acquired.tv_sec-starttime.tv_sec)+(acquired.tv_usec-starttime.tv_usec)*1e-6;
    for (int i=0;i<dests.count();i++) {
	char cbuf[10];
	dbg("World.sendMessages",6) << "Sending messages to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address addr = lo_address_new(dests.getHost(i), cbuf);
	if (sendstart) {
	    lo_send(addr,"/pf/started","");
	    lo_send(addr,"/pf/set/minx","f",-(float)MAXRANGE/1000.0);
	    lo_send(addr,"/pf/set/maxx","f",MAXRANGE/1000.0);
	    lo_send(addr,"/pf/set/miny","f",0.0);
	    lo_send(addr,"/pf/set/maxy","f",MAXRANGE/1000.0);
	}
	lo_send(addr,"/pf/frame","i",lastframe);
	// Handle entries
	std::set<int>exitids = lastid;
	unsigned int priornpeople=lastid.size();
	unsigned activePeople=0;
	for (std::vector<Person>::iterator p=people.begin();p!=people.end();p++){
	    if (p->getAge() >= AGETHRESHOLD) {
		activePeople++;
		exitids.erase(p->getID());
		if ( lastid.count(p->getID()) == 0)
		    lo_send(addr,"/pf/entry","ifii",lastframe,now,p->getID(),p->getChannel());
		lastid.insert(p->getID());
	    }
	}

	// Handle exits
	for (std::set<int>::iterator p=exitids.begin();p!=exitids.end();p++) {
	    lo_send(addr,"/pf/exit","ifi",lastframe,now,*p);
	    lastid.erase(*p);
	}

	// Current size
	if (activePeople != priornpeople)
	    lo_send(addr,"/pf/set/npeople","i",people.size());

	// Updates
	for (std::vector<Person>::iterator p=people.begin();p!=people.end();p++){
	    if (p->getAge() >= AGETHRESHOLD) {
		const Point *l =p->getLegs();
		float lspace=(l[1]-l[0]).norm();
		lo_send(addr, "/pf/update","ififfffffiii",lastframe,now,p->getID(),p->getPosition().X()/1000,p->getPosition().Y()/1000,p->getVelocity().X(),p->getVelocity().Y(),(lspace+p->getLegDiam())/1000,p->getLegDiam()/1000,0,0,p->getChannel());
	    }
	}
	lo_address_free(addr);
    }
}

mxArray *World::convertToMX() const {
    const char *fieldnames[]={"tracks","nextid","npeople","assignments","bestlike"};
    mxArray *world = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pNextid = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pNextid) = nextid;
    mxSetField(world,0,"nextid",pNextid);

    mxArray *pNpeople = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pNpeople) = people.size();
    mxSetField(world,0,"npeople",pNpeople);

    mxArray *pAssignments = mxCreateNumericMatrix(assignments.size(),2,mxINT32_CLASS,mxREAL);
    int *idata = (int *)mxGetPr(pAssignments);
    for (unsigned int i=0;i<assignments.size();i++)
	*idata=assignments[i];
    assert(legassigned.size()==assignments.size());
    for (unsigned int i=0;i<assignments.size();i++)
	*idata++=legassigned[i];
    mxSetField(world,0,"assignments",pAssignments);

    mxArray *pBestlike = mxCreateDoubleMatrix(bestlike.size(),2,mxREAL);
    double *data = mxGetPr(pBestlike);
    for (unsigned int i=0;i<bestlike.size();i++)
	*data++=bestlike[i];
    mxSetField(world,0,"bestlike",pBestlike);

    const char *pfieldnames[]={"id","position","legs","prevlegs","legvelocity","scanpts","posvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount"};
    mxArray *pPeople;
    if ((pPeople = mxCreateStructMatrix(1,people.size(),sizeof(pfieldnames)/sizeof(pfieldnames[0]),pfieldnames)) == NULL) {
	fprintf(stderr,"Unable to create people matrix of size (1,%ld)\n",people.size());
    }

    for (unsigned int i=0;i<people.size();i++) {
	people[i].addToMX(pPeople,i);
    }

    if (people.size()>0 && mxSetClassName(pPeople,"Person")) {
	fprintf(stderr,"Unable to convert people to a Matlab class\n");
    }

    mxSetField(world,0,"tracks",pPeople);					      

    if (mxSetClassName(world,"World")) {
	fprintf(stderr,"Unable to convert world to a Matlab class\n");
    }

    return world;
}

