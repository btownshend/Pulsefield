#include "vis.h"

Vis::Vis() {
}

void Vis::update(const SickIO *s) {
    sick=s;
    classifier.update(*sick);
}

mxArray *Vis::convertToMX() const {
    const char *fieldnames[]={"cframe","nmeasure","range","angle","frame","acquired","class","shadowed"};
    mxArray *vis = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pFrame = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pFrame) = sick->getFrame();
    mxSetField(vis,0,"frame",pFrame);

    mxArray *pCframe = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pCframe) = sick->getFrame();
    mxSetField(vis,0,"cframe",pCframe);

    mxArray *pNmeasure = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pNmeasure) = sick->getNumMeasurements();
    mxSetField(vis,0,"nmeasure",pNmeasure);

    mxArray *pacquired = mxCreateDoubleMatrix(1,1,mxREAL);
    struct timeval acquired=sick->getAcquired();
    *mxGetPr(pacquired) = (acquired.tv_sec + acquired.tv_usec/1e6)/86400.0 + 719529;
    mxSetField(vis,0,"acquired",pacquired);

    const mwSize dims[3]={sick->getNumEchoes(),1,sick->getNumMeasurements()};
    mxArray *pRange = mxCreateNumericArray(3,dims,mxDOUBLE_CLASS,mxREAL);
    assert(pRange!=NULL);
    double *data=mxGetPr(pRange);
    for (int j=0;j<sick->getNumEchoes();j++) {
	const unsigned int *range=sick->getRange(j);
	for (int i=0;i<sick->getNumMeasurements();i++)
	    *data++=range[i]/1000.0;
    }
    mxSetField(vis,0,"range",pRange);

    mxArray *pAngle = mxCreateDoubleMatrix(1,sick->getNumMeasurements(),mxREAL);
    assert(pAngle!=NULL);
    data=mxGetPr(pAngle);
    for (int i=0;i<sick->getNumMeasurements();i++)
	*data++=(i-(sick->getNumMeasurements()-1)/2.0)*sick->getScanRes()*M_PI/180;
    mxSetField(vis,0,"angle",pAngle);

    std::vector<unsigned int> classes=classifier.getclasses();
    mxArray *pCl = mxCreateNumericMatrix(classes.size(),1,mxUINT16_CLASS,mxREAL);
    assert(pCl!=NULL);
    unsigned short *sdata=(unsigned short *)mxGetPr(pCl);
    for (unsigned int i=0;i<classes.size();i++)
	*sdata++=classes[i];
    mxSetField(vis,0,"class",pCl);

    const std::vector<bool> shadowed[2]={ classifier.getshadowed(0), classifier.getshadowed(1)};
    mxArray *pSh = mxCreateLogicalMatrix(shadowed[0].size(),2);
    assert(pSh!=NULL);
    mxLogical *ldata=mxGetLogicals(pSh);
    for (unsigned int i=0;i<2;i++)
	for (unsigned int j=0;j<shadowed[i].size();j++)
	    *ldata++=shadowed[i][j];
    mxSetField(vis,0,"shadowed",pSh);

    return vis;
}
