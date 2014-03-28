#include "tracker.h"

Tracker::Tracker() {
}

void Tracker::track(const Vis *vis) {
    // TODO
}

mxArray *Tracker::convertToMX() const {
    const char *fieldnames[]={"tracks","debug"};
    mxArray *tracker = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pDebug = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pDebug) = 0;
    mxSetField(tracker,0,"debug",pDebug);

    if (mxSetClassName(tracker,"World")) {
	fprintf(stderr,"Unable to convert tracker to a Matlab class\n");
    }
    return tracker;

}
