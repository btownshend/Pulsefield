#include <iostream>
#include "bezier.h"
#include "dbg.h"

int main() {
    SetDebug("20");
    std::vector<Point> c(4);
    c[0]=Point(0.0,0.0);
    c[1]=Point(0.0,1.0);
    c[2]=Point(1.0,1.0);
    c[3]=Point(10.0,0.0);
    
    Bezier b(c);
    std::cout << "Length = " << b.getLength() << std::endl;
    for (int i=0;i<2;i++) {
	std::vector<Point> p;
	if (i==0) {
	    p=b.interpolate(5);
	    std::cout << "Interpolate(5) =  [";
	} else if (i==1) {
	    p=b.interpolate(1.0f);
	    std::cout << "Interpolate(0.1) =  [";
	}
	for (unsigned int i=0;i<p.size();i++)
	    std::cout << p[i] << " ";
	std::cout << std::endl;
    }
}
