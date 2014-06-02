#include <sys/time.h>

#include "attributes.h"

std::ostream &operator<<(std::ostream &s, const Attribute &c) {
    s << c.getValue() << "@" << c.getTime();
    return s;
}

std::ostream &operator<<(std::ostream &s, const Attributes &attributes) {
    for (std::map<std::string,Attribute>::const_iterator a=attributes.attrs.begin(); a!=attributes.attrs.end();a++) {
	s << a->first << ": " << a->second << " ";
    }
    return s;
}

std::vector<Point> Attributes::applyMovements(std::string attrname, float attrValue,const std::vector<Point> &pts) const {
    float v=TouchOSC::getValue(attrname,"amplitude",attrValue,0.0) * 0.5;
    float s=TouchOSC::getValue(attrname,"scale",attrValue,0.5) * 5;
    float ph=TouchOSC::getValue(attrname,"phase",attrValue,0.0)*1.0;
    float tx=TouchOSC::getValue(attrname,"temporalx",attrValue,0.0)*4.0;
    float ty=TouchOSC::getValue(attrname,"temporaly",attrValue,0.0)*4.0;
    dbg("Attributes.applyMovements",5) << "Value=" << v << ", Scale=" << s << ", Phase=" << ph << ", Temporal=" << tx << "," << ty << std::endl;
    struct timeval now;
    gettimeofday(&now,0);
    std::vector<Point> result(pts.size());
    for (int i=0;i<pts.size();i++) {
	if (i>0 && pts[i]==pts[i-1]) {
	    // Don't do anything to inserted blanking points 
	    result[i]=result[i-1];
	    continue;
	}
	Point np=pts[i]*s+ph;
	np=np+Point(tx,ty)*(now.tv_sec%1000+now.tv_usec/1e6);
	double noise1=Simplex::noise(np.X()*s,np.Y()*s)*v;
	double noise2=Simplex::noise(np.X(),-np.Y())*v;
	dbg("PointMovement.apply",10) << "noise=[" << noise1 << "," << noise2 << "]" << std::endl;
	result[i]=Point(pts[i].X()+noise1,pts[i].Y()+noise2);
    }
    return result;
}
