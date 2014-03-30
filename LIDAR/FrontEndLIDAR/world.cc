#include "world.h"

World::World() {
}

void World::track(const Vis *vis) {
    // TODO
}

mxArray *World::convertToMX() const {
    const char *fieldnames[]={"tracks","nextid"};
    mxArray *world = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pNextid = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pNextid) = 0;
    mxSetField(world,0,"nextid",pNextid);

    const char *pfieldnames[]={"id","position","legs","prevlegs","legvelocity","legclasses","posvar","velocity","legdiam","leftness","age","consecutiveInvisibleCount","totalVisibleCount"};
    mxArray *pPeople = mxCreateStructMatrix(1,people.size(),sizeof(pfieldnames)/sizeof(pfieldnames[0]),pfieldnames);

    for (unsigned int i=0;i<people.size();i++) {
	people[i].addToMX(pPeople,i);
    }

    if (mxSetClassName(pPeople,"Person")) {
	fprintf(stderr,"Unable to convert people to a Matlab class\n");
    }

    mxSetField(world,0,"tracks",pPeople);					      

    if (mxSetClassName(world,"World")) {
	fprintf(stderr,"Unable to convert world to a Matlab class\n");
    }
    return world;
}
