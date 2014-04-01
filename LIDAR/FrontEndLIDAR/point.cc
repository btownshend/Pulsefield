#include "point.h"

std::ostream& operator<<(std::ostream &s, const Point &p) {
    s << "(" << p.x << "," << p.y << ")";
    return s;
}

