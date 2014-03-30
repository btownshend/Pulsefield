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

    mxArray *pPeople=people.convertToMX();
    mxSetField(world,0,"tracks",pPeople);					      

    if (mxSetClassName(world,"World")) {
	fprintf(stderr,"Unable to convert world to a Matlab class\n");
    }
    return world;
}
