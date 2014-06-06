#include <sys/time.h>

#include "attributes.h"
#include "music.h"

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

std::vector<CPoint> Attributes::applyMovements(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    dbg("Attributes.applyMovements",5) << "Attribute " << attrname << " enable=" << TouchOSC::getEnabled(attrname,"noise") << std::endl;
    float v=TouchOSC::getValue(attrname,"noise-amp",attrValue,0.0) * 0.5;
    float s=TouchOSC::getValue(attrname,"noise-scale",attrValue,0.5) * 5;
    float ph=TouchOSC::getValue(attrname,"noise-phase",attrValue,0.0)*1.0;
    float tx=TouchOSC::getValue(attrname,"noise-time_x",attrValue,0.0)*4.0;
    float ty=TouchOSC::getValue(attrname,"noise-time_y",attrValue,0.0)*4.0;
    if (!TouchOSC::getEnabled(attrname,"noise"))
	return pts;
    dbg("Attributes.applyMovements",5) << "Value=" << v << ", Scale=" << s << ", Phase=" << ph << ", Temporal=" << tx << "," << ty << std::endl;
    struct timeval now;
    gettimeofday(&now,0);
    std::vector<CPoint> result(pts.size());
    for (int i=0;i<pts.size();i++) {
	Point np=pts[i]*s+ph;
	np=np+Point(tx,ty)*(now.tv_sec%1000+now.tv_usec/1e6);
	double noise1=Simplex::noise(np.X()*s,np.Y()*s)*v;
	double noise2=Simplex::noise(np.X(),-np.Y())*v;
	dbg("CPointMovement.apply",10) << "noise=[" << noise1 << "," << noise2 << "]" << std::endl;
	result[i]=CPoint(pts[i].X()+noise1,pts[i].Y()+noise2,pts[i].getColor());
    }
    return result;
}

std::vector<CPoint> Attributes::applyDashes(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    float onLength=TouchOSC::getValue(attrname,"dash-on",attrValue,0.0)*0.50;  // On-length in meters
    float offLength=TouchOSC::getValue(attrname,"dash-off",attrValue,0.5)*0.50;  // Off-length in meters
    float velocity=TouchOSC::getValue(attrname,"dash-vel",attrValue,0.0)*3.0;  // Dash-velocity in meters/s
    if (!TouchOSC::getEnabled(attrname,"dashes"))
	return pts;
    dbg("Attributes.applyDashes",5) << "On=" << onLength << ", off=" << offLength << ", vel=" << velocity << std::endl;
    struct timeval now;
    gettimeofday(&now,0);
    std::vector<CPoint> result=pts;
    float totallen=onLength+offLength;
    float dist=fmod(((now.tv_sec+now.tv_usec/1e6)*velocity),totallen);

    // TODO: Instead of blanking, put more points in 'on' region, less in 'off' region
    for (int i=1;i<result.size();i++) {
	if (dist<offLength)
	    result[i].setColor(Color(0,0.01,0));  // Turn off this segment (but don't use zero or it'll look like blanking)
	dist+=(result[i]-result[i-1]).norm();
	dist=fmod(dist,totallen);
    }
    return result;
}

float Attributes::getTotalLen(const std::vector<CPoint> &pts)  {
    float totalLen=0;
    for (int i=1;i<pts.size();i++) {
	totalLen += (pts[i]-pts[i-1]).norm();
    }
    return totalLen;
}

std::vector<CPoint> Attributes::applyMusic(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    float amplitude=TouchOSC::getValue(attrname,"music-amp",attrValue,0.0)*0.5;  // Amplitude
    float beat=TouchOSC::getValue(attrname,"music-beat",attrValue,1)*1.0;  // When in bar
    float dur=TouchOSC::getValue(attrname,"music-pulselen",attrValue,1)*1.0;  // Duration (pulse length) in bars
    float speed=TouchOSC::getValue(attrname,"music-speed",attrValue,1)*16.0;  // Duration (pulse length) in bars
    if (!TouchOSC::getEnabled(attrname,"music"))
	return pts;
    float fracbar=Music::instance()->getFracBar();
    dbg("Attributes.applyMusic",2) << "Amp=" << amplitude << ", beat=" << beat << ", dur=" << dur << ",speed=" << speed << ", fracBar=" << fracbar << std::endl;
    std::vector<CPoint> result=pts;
    float totalLen=getTotalLen(pts);

    float len=0;
    for (int i=1;i<result.size();i++) {
	len += (result[i]-result[i-1]).norm();
	float frac=fmod(len/totalLen+beat,1.0);
	Point vec=pts[i]-pts[i-1]; vec=vec/vec.norm();
	Point ortho=Point(-vec.Y(),vec.X());
	if (abs(frac-fracbar*speed)<dur)
	    result[i]=result[i]+ortho*amplitude;
    }
    return result;
}

std::vector<CPoint> Attributes::applyStraighten(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    if (!TouchOSC::getEnabled(attrname,"straighten"))
	return pts;
    float maxLen=std::max(fabs(pts.back().X()-pts.front().X()),fabs(pts.back().Y()-pts.front().Y()));
    float minTurn=TouchOSC::getValue(attrname,"straighten",attrValue,0.0)*maxLen/2;  // Min distance before turning
    dbg("Attributes.applyStraighten",2) << "maxLen=" << maxLen << ",minTurn=" << minTurn << std::endl;
    std::vector<CPoint> result=pts;
    float runlen=minTurn;
    int dir=0;
    for (int i=1;i<result.size();i++) {
	Point delta=pts[i]-result[i-1];
	if (runlen>=std::min(minTurn,(pts.back()-result[i-1]).norm())) {
	    // Choose direction
	    int newdir=0;
	    if (fabs(delta.X())<fabs(delta.Y()))
		newdir=1;
	    if (newdir!=dir) {
		dbg("Attributes.applyStraighten",2) << "Turn at point " << result[i-1] << "->" << result[i] << " with runlen=" << runlen << std::endl;
		runlen=0;
		dir=newdir;
	    }
	}
	if (dir==0)
	    result[i]=result[i-1]+Point(delta.X(),0.0);
	else
	    result[i]=result[i-1]+Point(0.0,delta.Y());
	runlen+=(result[i]-result[i-1]).norm();
    }
    return result;
}

std::vector<CPoint> Attributes::apply(std::vector<CPoint> pts) const {
    dbg("Attributes.apply",2) << "Applying " << attrs.size() << " attributes to " << pts.size() << " points" << std::endl;
    for (std::map<std::string,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	pts=applyStraighten(a->first,a->second.getValue(),pts);
	pts=applyMovements(a->first,a->second.getValue(),pts);
	pts=applyDashes(a->first,a->second.getValue(),pts);
	pts=applyMusic(a->first,a->second.getValue(),pts);
    }
    return pts;
}
