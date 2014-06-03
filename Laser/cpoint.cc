#include <iostream>
#include <assert.h>
#include "cpoint.h"

std::ostream& operator<<(std::ostream &s, const CPoint &p) {
    s << (Point)p << p.c;
    return s;
}
