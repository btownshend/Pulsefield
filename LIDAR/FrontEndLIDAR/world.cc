#include "world.h"

World::World() {
}

void World::track(const Vis *vis) {
    // TODO
}

mxArray *World::convertToMX() const {
    const char *fieldnames[]={"tracks","debug"};
    mxArray *world = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pDebug = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pDebug) = 0;
    mxSetField(world,0,"debug",pDebug);

    if (mxSetClassName(world,"World")) {
	fprintf(stderr,"Unable to convert world to a Matlab class\n");
    }
    return world;

}
