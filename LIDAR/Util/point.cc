#include <istream>
#include <assert.h>
#include "point.h"

std::ostream& operator<<(std::ostream &s, const Point &p) {
    s << "(" << p.x << "," << p.y << ")";
    return s;
}

std::istream& operator>>(std::istream &s,  Point &p) {
    char lparen,rparen,comma;
    s.get(lparen);
    s>>p.x;
    s.get(comma);
    s>>p.y;
    s.get(rparen);
    assert(lparen=='(' && comma==',' && rparen == ')');
    return s;
}


