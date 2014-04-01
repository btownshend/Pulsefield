#include "world.h"
#include "likelihood.h"
#include "vis.h"

World::World() {
    lastframe=0;
    nextid=1;
}

void World::print() const {
    printf("World: frame=%d\n", lastframe);
    for (unsigned int i=0;i<people.size();i++) {
	people[i].print();
    }
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
	dbg("Likelihood",2) << "Frame %d likelihoods: \n" << likes;
    }

    // Greedy assignment
    Likelihood result=likes.smartassign();
    
    // Implement assignment
    for (int i=0;i<result.size();i++) {
	Assignment a=result[i];
	dbg("Assign",2) << a;
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

    print();
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

