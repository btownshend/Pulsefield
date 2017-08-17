#include <dbg.h>
#include "outsiders.h"

void Outsiders::update(Point w) {
    // Have a LIDAR hit at world point w during frame
    w=w-center;  // Offset to center
    float dist=w.norm();
    if (dist>=innerRadius && dist<=outerRadius) {
	// in the donut
	float angle=w.getTheta()+M_PI;
	int bin=(int)(angle*nDivisions/(2*M_PI));
	assert(bin>=0 && bin<nDivisions);
	lastHit[bin]=curFrame;
    }
}

void Outsiders::update(int frame, const SickIO *s) {
    curFrame=frame;
    for (int i=0;i<s->getNumMeasurements();i++)
	update(s->getWorldPoint(i));
}

void Outsiders::dump() const {
    char s[nDivisions+1];
    for (int i=0;i<nDivisions;i++)  {
	if (lastHit[i]>=curFrame-life)
	    s[i]='+';
	else
	    s[i]='-';
    }
    s[nDivisions]=0;
    dbg("Outsiders.dump",1) << "outsiders=" << s << std::endl;
}

void Outsiders::sendMessages(const char *host, int port) const {
    dbg("Outsiders.sendMessages",2) << "Sending  messages to " << host << ":" << port << std::endl;
    char cbuf[10];
    sprintf(cbuf,"%d",port);
    lo_address addr = lo_address_new(host, cbuf);

    dump();

    // Send bits of outsiders
    int nvals=(int)((nDivisions+31)/32);
    int nmsgs=(int)((nvals+3)/4);
    nvals=nmsgs*4;
    int v[nvals];
    for (int i=0;i<nvals;i++)
	v[i]=0;
    for (int i=0;i<nDivisions;i++)  {
	if (lastHit[i]>=curFrame-life)
	    v[(int)(i/32)] |=1<<(i%32);
    }
    for (int i=0;i<nvals;i+=4) {
	lo_send(addr,"/pf/outsiders","iiiiii",i,nDivisions,v[i],v[i+1],v[i+2],v[i+3]);
	dbg("Outsiders.sendMessages",1) << "v=[" << v[i] << "," << v[i+1] << "," << v[i+2] << "," << v[i+3] << std::endl;
    }

    // Done!
    lo_address_free(addr);
}
