#include <iostream>
#include <math.h>

#include "etherdream.h"
#include "laser.h"
#include "dbg.h"
#include "point.h"

int Laser::startBlank=10;

Laser::Laser() {
    static const int NP=100;
    drawCircle(Point(0,0),20000,NP);
    drawCircle(Point(0,0),10000,NP);
    drawCircle(Point(10000,0),10000,NP);
    drawCircle(Point(-10000,0),10000,NP);
}

void Laser::setPoints(const std::vector<etherdream_point> &_points) {
    dbg("Laser.setPoint",2) << "Setting to " << _points.size() << " new points" << std::endl;
    points=_points;
    update();
}

int Laser::open() {
    etherdream_lib_start();

    /* Sleep for a bit over a second, to ensure that we see broadcasts
     * from all available DACs. */
    usleep(1200000);

    int cc = etherdream_dac_count();
    if (!cc) {
	printf("No DACs found.\n");
	return -1;
    }

    int i;
    for (i = 0; i < cc; i++) {
	printf("%d: Ether Dream %06lx\n", i,etherdream_get_id(etherdream_get(i)));
    }

    d = etherdream_get(0);
    printf("Connecting...\n");
    if (etherdream_connect(d) < 0)
	return -1;
    return 0;
}

void Laser::clear() {
    dbg("Laser.clear",2) << "clear()" << std::endl;
    points.clear();
}

uint16_t colorsin(float pos) {
	int res = (sin(pos) + 1) * 32768;
	if (res < 0) return 0;
	if (res > 65535) return 65535;
	return res;
}

void Laser::drawCircle(Point center, float r, int npts) {
    dbg("Laser.drawCircle",2) << "drawCircle(" << center << "," << r << "," << npts << ")" << std::endl;
    int oldsize=points.size();
    points.resize(points.size()+npts+startBlank);
    struct etherdream_point *pt = &points[oldsize];
    for (int i=0;i<startBlank;i++,pt++) {
	pt->x=center.X();
	pt->y=r+center.Y();
	pt->g=0;
	pt->r=0;
	pt->b=0;
    }
    for (int i = 0; i < npts; i++,pt++) {
	float ip = i * 2.0 * M_PI /npts;
	pt->x = sin(ip) * r + center.X(); 
	pt->y = cos(ip) * r + center.Y(); 
	pt->g = 65535;
	pt->r=pt->g;
	pt->b=pt->g;
    }
}

void Laser::update() {
    if (d==0) {
	std::cerr << "Laser not open" << std::endl;
	return;
    }
    dbg("Laser.update",1) << "Wait for ready." << std::endl;
    etherdream_wait_for_ready(d);
    dbg("Laser.update",1) << "Sending " << points.size() << " points" << std::endl;
    int res = etherdream_write(d,points.data(), points.size(), PPS, -1);
    if (res != 0)
	printf("write %d\n", res);
}
