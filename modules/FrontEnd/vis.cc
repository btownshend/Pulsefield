#include "vis.h"
#include "parameters.h"

Vis::Vis() {
}

void Vis::update(const SickIO *s) {
    sick=s;
}

#ifdef MATLAB
mxArray *Vis::convertToMX(int frame) const {
    const char *fieldnames[]={"cframe","nmeasure","range","angle","frame","acquired","unit","scantime","world","origin","rotation"};
    mxArray *vis = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pFrame = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pFrame) = frame;
    mxSetField(vis,0,"frame",pFrame);

    mxArray *pUnit = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pUnit) = sick->getId();
    mxSetField(vis,0,"unit",pUnit);

    mxArray *pCframe = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pCframe) = sick->getScanCounter();
    mxSetField(vis,0,"cframe",pCframe);

    mxArray *pNmeasure = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pNmeasure) = sick->getNumMeasurements();
    mxSetField(vis,0,"nmeasure",pNmeasure);

    mxArray *pacquired = mxCreateDoubleMatrix(1,1,mxREAL);
    struct timeval acquired=sick->getAcquisitionTime();
    *mxGetPr(pacquired) = (acquired.tv_sec + acquired.tv_usec/1e6)/86400.0 + 719529;
    mxSetField(vis,0,"acquired",pacquired);

    mxArray *pscantime = mxCreateDoubleMatrix(1,1,mxREAL);
    struct timeval scantime=sick->getAbsScanTime();
    *mxGetPr(pscantime) = (scantime.tv_sec + scantime.tv_usec/1e6)/86400.0 + 719529;
    mxSetField(vis,0,"scantime",pscantime);

    const mwSize dims[3]={sick->getNumEchoes(),1,sick->getNumMeasurements()};
    mxArray *pRange = mxCreateNumericArray(3,dims,mxDOUBLE_CLASS,mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (unsigned int j=0;j<sick->getNumEchoes();j++) {
	const unsigned int *range=sick->getRange(j);
	for (unsigned int i=0;i<sick->getNumMeasurements();i++)
	    *data++=range[i]/UNITSPERM;
    }
    mxSetField(vis,0,"range",pRange);

    mxArray *pAngle = mxCreateDoubleMatrix(1,sick->getNumMeasurements(),mxREAL);
    assert(pAngle!=NULL);
    data=mxGetPr(pAngle);
    for (unsigned int i=0;i<sick->getNumMeasurements();i++)
	*data++=(i-(sick->getNumMeasurements()-1)/2.0)*sick->getScanRes()*M_PI/180;
    mxSetField(vis,0,"angle",pAngle);

    mxArray *pWorld = mxCreateDoubleMatrix(sick->getNumMeasurements(),2,mxREAL);
    assert(pWorld!=NULL);
    data=mxGetPr(pWorld);
    for (unsigned int i=0;i<sick->getNumMeasurements();i++)
	*data++=sick->getWorldPoint(i).X()/UNITSPERM;
    for (unsigned int i=0;i<sick->getNumMeasurements();i++)
	*data++=sick->getWorldPoint(i).Y()/UNITSPERM;
    mxSetField(vis,0,"world",pWorld);

    mxArray *pOrigin = mxCreateDoubleMatrix(1,2,mxREAL);
    assert(pOrigin!=NULL);
    data=mxGetPr(pOrigin);
    *data++=sick->getOrigin().X()/UNITSPERM;
    *data++=sick->getOrigin().Y()/UNITSPERM;
    mxSetField(vis,0,"origin",pOrigin);

    mxArray *pRotation = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pRotation) = sick->getCoordinateRotationDeg();
    mxSetField(vis,0,"rotation",pRotation);
    
    return vis;
}
#endif
