#include "target.h"

Point Target::getCenter() const {
    Point cp=points[0];
    for (unsigned int i=1;i<points.size();i++)
	cp=cp+points[i];
    cp=cp/points.size();
    return cp;
}

