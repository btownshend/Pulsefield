#include <iostream>
#include <istream>
#include <assert.h>
#include "point.h"

std::ostream& operator<<(std::ostream &s, const Point &p) {
    s << "(" << p.X() << "," << p.Y() << ")";
    return s;
}

std::istream& operator>>(std::istream &s,  Point &p) {
    char lparen,rparen,comma;
    do {
	s.get(lparen);
    } while (isspace(lparen) && s.good());
    if (lparen!='(') {
	std::cerr << "Failed to read ( from stream (got '" << lparen << "'=" << (int)lparen << "), s.good=" << s.good() << std::endl;
	s.setstate(std::ios::failbit);
	return s;
    }

    float nx;
    s>>nx;
    p.setX(nx);
    do {
	s.get(comma);
    } while (isspace(comma) && s.good());
    if (comma!=',') {
	std::cerr << "Failed to read , from stream (got " << (int)comma << ")" << std::endl;
	s.setstate(std::ios::failbit);
	return s;
    }
    float ny;
    s>>ny;
    p.setY(ny);
    do {
	s.get(rparen);
    } while(isspace(rparen) && s.good());
    if (rparen!=')') {
	std::cerr << "Failed to read ) from stream (got " << (int)rparen << ")" << std::endl;
	s.setstate(std::ios::failbit);
	return s;
    }
    
    return s;
}


