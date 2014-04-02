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

void World::track(const Targets &targets, const Vis &vis, int frame, float fps) {
    int nsteps;
    if (lastframe>0)
	nsteps=frame-lastframe;
    else
	nsteps=1;
    lastframe=frame;

    // Update existing tracks with next prediction
    for (unsigned int i=0;i<people.size();i++)
	people[i].predict(nsteps,fps);

    // Assign current classes to tracks
    Likelihood likes;
    for (unsigned int i=0;i<people.size();i++)
	people[i].getclasslike(targets,vis,likes,i);

    // Compute likelihoods of new tracks
    Person::newclasslike(targets,vis,likes);

    if (likes.size()>0) {
	dbg("Likelihood",2) << "Frame " << frame << " likelihoods: \n" << likes;
    }

    // Greedy assignment
    Likelihood result=likes.smartassign();
    
    // Implement assignment
    for (int i=0;i<result.size();i++) {
	Assignment a=result[i];
	if (a.track<0) {
	    people.push_back(Person(nextid, vis, a.target1, a.target2));
	    nextid++;
	} else
	    people[a.track].update(vis,a.target1,a.target2,nsteps,fps);
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
    const char *fieldnames[]={"tracks","nextid","npeople"};
    mxArray *world = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pNextid = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pNextid) = nextid;
    mxSetField(world,0,"nextid",pNextid);

    mxArray *pNpeople = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pNpeople) = people.size();
    mxSetField(world,0,"npeople",pNpeople);

    const char *pfieldnames[]={"id","position","legs","prevlegs","legvelocity","legclasses","posvar","velocity","legdiam","leftness","age","consecutiveInvisibleCount","totalVisibleCount"};
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

