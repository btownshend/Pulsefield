#include "people.h"

People::People() {
    ;
}

mxArray *People::convertToMX() const {
    const char *fieldnames[]={"id","position","legs","prevlegs","legvelocity","legclasses","posvar","velocity","legdiam","leftness","age","consecutiveInvisibleCount","totalVisibleCount"};
    mxArray *people = mxCreateStructMatrix(1,persons.size(),sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    for (unsigned int i=0;i<persons.size();i++) {
	persons[i].addToMX(people,i);
    }

    if (mxSetClassName(people,"Person")) {
	fprintf(stderr,"Unable to convert people to a Matlab class\n");
    }
    return people;
}
