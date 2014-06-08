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
    static const int numShapes=6;
    float amplitude=TouchOSC::getValue(attrname,"music-amp",attrValue,0.0)*0.5;  // Amplitude
    float beat=TouchOSC::getValue(attrname,"music-beat",attrValue,1)*1.0;  // When in bar
    float length=TouchOSC::getValue(attrname,"music-pulselen",attrValue,0.2)*1.0;  // Length in fraction of line length
    float speed=TouchOSC::getValue(attrname,"music-speed",attrValue,1)*15.0+1.0;  // Speed factor
    int shape=(int)(TouchOSC::getValue(attrname,"music-shape",attrValue,1)*numShapes);
    if (!TouchOSC::getEnabled(attrname,"music"))
	return pts;
    float fracbar=Music::instance()->getFracBar();
		    dbg("Attributes.applyMusic",2) << "Amp=" << amplitude << ", beat=" << beat << ", length=" << length << ",speed=" << speed << ", length=" << length << ", fracBar=" << fracbar << ", shape=" << shape <<  std::endl;
    std::vector<CPoint> result=pts;
    float totalLen=getTotalLen(pts);

    float len=0;
    for (int i=1;i<result.size();i++) {
	Point vec=pts[i]-pts[i-1];
	float veclen=vec.norm();
	vec=vec/veclen;
	len += veclen;
	float frac=fmod(len/totalLen/speed+beat,1.0);
	Point ortho=Point(-vec.Y(),vec.X());
	float shapepos=fmod(frac-fracbar,1.0)/(length/speed);   // Varies from -1 to +1 over shape
	if (shapepos>-1 && shapepos<1) {
	    float shapeval=0;
	    switch (shape) {
	    case 0:
		shapeval=shapepos>0?0.0:1.0;
		break;
	    case 1:
		shapeval=1-fabs(-shapepos);
		break;
	    case 2:
		shapeval=sin(shapepos*M_PI);
		break;
	    case 3:
		shapeval=Simplex::noise(pts[i].X()*5,pts[i].Y()*5);
		break;
	    case 4:
		shapeval=Simplex::noise(pts[i].X()*10,pts[i].Y()*10);
		break;
	    case 5:
	    default:
		shapeval=1;
		break;
	    }
	    float offset=amplitude*shapeval;
	    dbg("Attributes.applyMusic",5) << "i=" << i << ", frac=" << frac << ", ortho=" << ortho << ", shapepos=" << shapepos << ", offset=" << offset << std::endl;
	    result[i]=result[i]+ortho*offset;
	} else {
	    dbg("Attributes.applyMusic",5) << "i=" << i << ", frac=" << frac << ", ortho=" << ortho << std::endl;
	}
    }
    return result;
}

std::vector<CPoint> Attributes::applyStraighten(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    if (!TouchOSC::getEnabled(attrname,"straighten"))
	return pts;
    int  nTurns=(int)(TouchOSC::getValue(attrname,"straighten",attrValue,0.1)*10)+1;  // Number of turns
    Point delta = pts.back()-pts.front();
    float minDist=std::min(fabs(delta.X()),fabs(delta.Y()));
    float totalLen=getTotalLen(pts);
    float blockSize=std::max(totalLen/nTurns/8,minDist/nTurns);
    dbg("Attributes.applyStraighten",2) << "nTurns=" << nTurns << ", delta=" << delta << ",blockSize=" << blockSize << std::endl;
    assert(blockSize>0);
    std::vector<CPoint> results;
    results.push_back(pts.front());
    // Draw manhattan path with blockSize blocks (in meters)
    float len=0;
    for (int i=1;i<pts.size();) {
	Point delta=pts[i]-results.back();
	if (fabs(delta.X())>=blockSize-.001)  {
	    results.push_back(results.back()+Point(copysign(blockSize,delta.X()),0));
	    len=0;
	} else if (fabs(delta.Y())>=blockSize-.001) {
	    results.push_back(results.back()+Point(0,copysign(blockSize,delta.Y())));
	    len = 0;
	} else if (len>blockSize*1.41) {
	    // In case we're going in circles, emit every now and then
	    results.push_back(pts[i]);
	    len=0;
	} else {
	    i++;
	    len+=delta.norm();
	}
    }
    CPoint priorpt=pts.back();
    priorpt.setX(results.back().X());
    results.push_back(priorpt);
    results.push_back(pts.back());
    results=CPoint::resample(results,pts.size());
    return results;
}

std::vector<CPoint> Attributes::applyDoubler(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const {
    if (!TouchOSC::getEnabled(attrname,"double"))
	return pts;
    if (pts.size()<2)
	return pts;
    int  nCopies=(int)(TouchOSC::getValue(attrname,"dbl-ncopies",attrValue,0)*4)+2;  // Number of copies
    float offset=TouchOSC::getValue(attrname,"dbl-offset",attrValue,0)*0.1;  // Offset of copies
    if (nCopies == 1)
	return pts;
    std::vector<CPoint> fwd;
    std::vector<CPoint> rev;
    for (int i=0;i<pts.size();i+=nCopies) {
	fwd.push_back(pts[i]);
	rev.push_back(pts[pts.size()-1-i]);
    }
    fwd.push_back(pts.back());
    rev.push_back(pts.front());

    std::vector<CPoint> results=fwd;
    for (int i=0;i<nCopies-1;i++) {
	std::vector<CPoint> src;
	if (i%2==0)
	    src=rev;
	else
	    src=fwd;

	results.push_back(src[0]);
	for (int j=1;j<src.size();j++) {
	    Point vec=src[j]-src[j-1];
	    if (vec.norm()==0)
		vec=Point(1,0);
	    else
		vec=vec/vec.norm();
	    Point ortho=Point(-vec.Y(),vec.X());
	    if ((pts.front()-pts.back()).norm() < .02) {
		if (j<5)
		    ortho=ortho*(j/5.0);
		if (src.size()-j-1 < 5)
		    ortho=ortho*((src.size()-j-1)/5.0);
	    }
	    results.push_back(src[j]+ortho*offset*(int)(i/2+1));
	}
    }
    dbg("Attributes.applyDoubler",2) << "nCopies=" << nCopies << " offset=" << offset << "  increased npoints from " << pts.size() << " to " << results.size() << std::endl;
    return results;
}

std::vector<CPoint> Attributes::apply(std::vector<CPoint> pts) const {
    if (!TouchOSC::instance()->isAttrsEnabled()) {
	dbg("Attributes.apply",2) << "Atrributes disabled" << std::endl;
	return pts;
    }
    bool applied[5] = {false,false,false,false,false};
    dbg("Attributes.apply",2) << "Applying " << attrs.size() << " attributes to " << pts.size() << " points" << std::endl;
    for (std::map<std::string,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	std::string attrname=a->first;
	if ( TouchOSC::getEnabled(attrname,"disable")) {
	    dbg("Attributes.apply",2) << "Attribute " << attrname << " disabled" << std::endl;
	    if (false && attrs.size()==1) {
		// Remove line
		dbg("Attributes.apply",2) << "Disabling connection since " << attrname << " is disabled and it is the only attribute" << std:: endl;
		return std::vector<CPoint>();
	    }
	} else {
	    if (!applied[0]) {
		pts=applyDoubler(attrname,a->second.getValue(),pts);
		//		applied[0]=true;
	    }
	    if (!applied[1]) {
		pts=applyStraighten(attrname,a->second.getValue(),pts);
		//		applied[1]=true;
	    }
	    if (!applied[2]) {
		pts=applyMovements(attrname,a->second.getValue(),pts);
		//		applied[2]=true;
	    }
	    if (!applied[3]) {
		pts=applyDashes(attrname,a->second.getValue(),pts);
		//		applied[3]=true;
	    }
	    if (!applied[4]) {
		pts=applyMusic(attrname,a->second.getValue(),pts);
		//		applied[4]=true;
	    }
	}
    }
    return pts;
}

// Keep only attributes >= minval
Attributes Attributes::filter(float minval) const { 
    Attributes result;
    for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	if (a->second.getValue() >= minval)
	    result.set(a->first,a->second);
    }
    return result;
}

// Get maximum attribute value (excluding fusion)
float Attributes::getMaxVal() const {
    float result=0;
    for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	if (a->second.getValue() >= result)
	    result=a->second.getValue();
    }
    return result;
}

// Reduce to the single strongest attribute
Attributes Attributes::keepStrongest() const {
    Attributes result;
    float maxval=getMaxVal();
    for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	if (a->second.getValue() == maxval) {
	    result.set(a->first,a->second);
	    break;
	}
    }
    return result;
}

