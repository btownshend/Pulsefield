#include <string.h>

#include "person.h"

Person::Person() {
}

void Person::addToMX(mxArray *people, int index) const {
    mxArray *pId = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *mxGetPr(pId) = id;
    mxSetField(people,index,"id",pId);

    mxArray *pPosition = mxCreateNumericMatrix(2,2,mxUINT32_CLASS,mxREAL);
    int *data = (int *)mxGetPr(pPosition);
    memcpy(data,position,sizeof(position));
    mxSetField(people,index,"position",pPosition);
}
