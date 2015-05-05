#include <sys/time.h>

#include "attributes.h"
#include "music.h"
#include "svg.h"
#include "drawing.h"

std::ostream &operator<<(std::ostream &s, const Attribute &c) {
    s << c.getValue() << "@" << c.getTime();
    return s;
}

void Attributes::drawLabels(Drawing &d, Point p1, Point p2) const {
    static const float DISPLAYTIME=10.0f;    // Time to display an attribute
    static const float CYCLETIME=1.0f; 	     // Time to cycle between display of multiple attributes
    float cycleTime=0;
    int nattrs=0;
    for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	if (a->first.find("all")==0)
	    // Skip all* attributes
	    continue;
	if (a->second.getTime() < DISPLAYTIME) {
	    nattrs++;
	    if (nattrs==1)
		cycleTime=a->second.getTime();   // Use for cycling through multiple labels
	}
    }
    if (nattrs==0)
	return;
    if (nattrs>1) {
	struct timeval now;
	gettimeofday(&now,0);
	cycleTime=(now.tv_sec%1000)+now.tv_usec/1e6;
    }
    
    dbg("Attributes.drawLabel",1) << "Have " << nattrs << " labels to draw at " << p1 << "-" << p2 << ", cycleTime=" << cycleTime <<  std::endl;
    int attrCntr=0;
    for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	if (a->first.find("all")==0)
	    // Skip all* attributes
	    continue;
	dbg("Attributes.drawLabel",3) << "Have label for " << a->first << " with time "  << a->second.getTime() << " at " << p1 << "-" << p2 << std::endl;
	if (a->second.getTime() < DISPLAYTIME) {
	    if (nattrs == 1 || ((int)(cycleTime/CYCLETIME))%nattrs == attrCntr) {  // Cycle through multiple labels every CYCLETIME seconds
		static const std::string SVGDIRECTORY("../SVGFiles");
		Point origin = (p1+p2)/2;
		float scaling=(p2-p1).norm();
		float rotateDeg=(p2-p1).getTheta()*180/M_PI+90;
		if (rotateDeg>180)
		    rotateDeg-=360;
		if (rotateDeg<-180)
		    rotateDeg+=360;
		// // Make it approximately oriented correclty
		// if (rotateDeg<-90)
		//     rotateDeg+=180;
		// // Make it approximately oriented correclty
		// if (rotateDeg>90)
		//     rotateDeg-=180;
		std::string filename=a->first+".svg";
		dbg("Attributes.drawLabel",2) << "svgfile(" << filename << ", " << origin << ", " << scaling << ", " << rotateDeg << ")" << std::endl;
		std::shared_ptr<SVG> s=SVGs::get(SVGDIRECTORY+"/"+filename);
		if (s!=nullptr) {
		    s->addToDrawing(d,origin,scaling,rotateDeg,Color(1.0,1.0,1.0));
		    break;   // Done once we've drawn a label
		} else {
		    dbg("Attributes.drawLabel",0) << "Missing SVG file: " << filename << std::endl;
		}
	    }
	    attrCntr++;
	}
    }
}

std::ostream &operator<<(std::ostream &s, const Attributes &attributes) {
    for (std::map<std::string,Attribute>::const_iterator a=attributes.attrs.begin(); a!=attributes.attrs.end();a++) {
	s << a->first << ": " << a->second << " ";
    }
    return s;
}

CPoints Attributes::applyMovements(std::string attrname, float attrValue,const CPoints &pts) const {
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
    CPoints result;
    for (int i=0;i<pts.size();i++) {
	Point np=pts[i]*s+ph;
	np=np+Point(tx,ty)*(now.tv_sec%1000+now.tv_usec/1e6);
	double noise1=Simplex::noise(np.X()*s,np.Y()*s)*v;
	double noise2=Simplex::noise(np.X(),-np.Y())*v;
	dbg("CPointMovement.apply",10) << "noise=[" << noise1 << "," << noise2 << "]" << std::endl;
	result.push_back(CPoint(pts[i].X()+noise1,pts[i].Y()+noise2,pts[i].getColor()));
    }
    return result;
}

CPoints Attributes::applyDashes(std::string attrname, float attrValue,const CPoints &pts) const {
    float onLength=TouchOSC::getValue(attrname,"dash-on",attrValue,0.0)*0.50;  // On-length in meters
    float offLength=TouchOSC::getValue(attrname,"dash-off",attrValue,0.5)*0.50;  // Off-length in meters
    float velocity=TouchOSC::getValue(attrname,"dash-vel",attrValue,0.0)*3.0;  // Dash-velocity in meters/s
    if (!TouchOSC::getEnabled(attrname,"dashes"))
	return pts;
    dbg("Attributes.applyDashes",5) << "On=" << onLength << ", off=" << offLength << ", vel=" << velocity << std::endl;
    struct timeval now;
    gettimeofday(&now,0);
    CPoints result=pts;
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

float Attributes::getTotalLen(const CPoints &pts)  {
    return pts.getLength();
}

CPoints Attributes::applyMusic(std::string attrname, float attrValue,const CPoints &pts) const {
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
    CPoints result=pts;
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

CPoints Attributes::applyStraighten(std::string attrname, float attrValue,const CPoints &pts) const {
    if (!TouchOSC::getEnabled(attrname,"straighten"))
	return pts;
    int  nTurns=(int)(TouchOSC::getValue(attrname,"straighten",attrValue,0.1)*10)+1;  // Number of turns
    Point delta = pts.back()-pts.front();
    float minDist=std::min(fabs(delta.X()),fabs(delta.Y()));
    float totalLen=getTotalLen(pts);
    float blockSize=std::max(totalLen/nTurns/8,minDist/nTurns);
    dbg("Attributes.applyStraighten",2) << "nTurns=" << nTurns << ", delta=" << delta << ",blockSize=" << blockSize << std::endl;
    if (blockSize<=0) {
	dbg("Attributes.applyStraighten",0) << "BAD blocksize: nTurns=" << nTurns << ", delta=" << delta << ",blockSize=" << blockSize << std::endl;
	return pts;
    }
    CPoints results;
    results.push_back(pts.front());
    // Draw manhattan path with blockSize blocks (in meters)
    float len=0;
    for (int i=1;i<pts.size();) {
	Point delta=pts[i]-results.back();
	if (fabs(delta.X())>blockSize/2)  {
	    CPoint tmp=results.back()+Point(copysign(blockSize,delta.X()),0);
	    tmp.setColor(pts[i].getColor());
	    results.push_back(tmp);
	    dbgn("Attributes.applyStraighten",3) << "x";
	    len=0;
	} else if (fabs(delta.Y())>blockSize/2) {
	    CPoint tmp=results.back()+Point(0,copysign(blockSize,delta.Y()));
	    tmp.setColor(pts[i].getColor());
	    results.push_back(tmp);
	    dbgn("Attributes.applyStraighten",3) << "y";
	    len = 0;
	} else if (len>blockSize*1.41) {
	    // In case we're going in circles, emit every now and then
	    results.push_back(pts[i]);
	    len=0;
	    dbgn("Attributes.applyStraighten",3) << "d";
	} else {
	    i++;
	    len+=(pts[i]-pts[i-1]).norm();
	    dbgn("Attributes.applyStraighten",3) << "-";
	}
	if (results.size()>5000) {
	    dbg("Attributes.applyStraighten",0) << "Infinite loop in applyStraighten: delta=" << delta << ", blockSize=" << blockSize << std::endl;
	    break;
	}
    }
    dbgn("Attributes.applyStraighten",3) << std::endl;
    CPoint priorpt=pts.back();
    priorpt.setX(results.back().X());
    results.push_back(priorpt);
    results.push_back(pts.back());
    if (results.size() >= 3 && pts.size()>=3) {
	dbg("Attributes.applyStraighten",10) << "     orig[0,1,2,..." << pts.size()-1 << "]=" << pts[0] << "," << pts[1] << "," << pts[2] << std::endl;
	dbg("Attributes.applyStraighten",10) << "results[0,1,2,..." << results.size()-1 << "]=" << results[0] << "," << results[1] << "," << results[2] << std::endl;
    }
    results=results.resample(pts.getLength()/pts.size());  // Resample uniformly at same spacing as original
    if (results.size() >= 3) {
	dbg("Attributes.applyStraighten",10) << "resamps[0,1,2,..." << results.size()-1 << "]=" << results[0] << "," << results[1] << "," << results[2] << std::endl;
    }
    return results;
}

CPoints Attributes::applyDoubler(std::string attrname, float attrValue,const CPoints &pts) const {
    if (!TouchOSC::getEnabled(attrname,"double"))
	return pts;
    if (pts.size()<2)
	return pts;
    int  nCopies=(int)(TouchOSC::getValue(attrname,"dbl-ncopies",attrValue,0)*4)+2;  // Number of copies
    float offset=TouchOSC::getValue(attrname,"dbl-offset",attrValue,0)*0.1;  // Offset of copies
    if (nCopies == 1)
	return pts;
    CPoints fwd;
    CPoints rev;
    for (int i=0;i<pts.size();i+=nCopies) {
	fwd.push_back(pts[i]);
	rev.push_back(pts[pts.size()-1-i]);
    }
    fwd.push_back(pts.back());
    rev.push_back(pts.front());

    CPoints results=fwd;
    for (int i=0;i<nCopies-1;i++) {
	CPoints src;
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

CPoints Attributes::apply(const CPoints &pts) const {
    CPoints result=pts;
    
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
		return CPoints();
	    }
	} else {
	    if (!applied[0]) {
		result=applyDoubler(attrname,a->second.getValue(),result);
		//		applied[0]=true;
	    }
	    if (!applied[1]) {
		result=applyStraighten(attrname,a->second.getValue(),result);
		//		applied[1]=true;
	    }
	    if (!applied[2]) {
		result=applyMovements(attrname,a->second.getValue(),result);
		//		applied[2]=true;
	    }
	    if (!applied[3]) {
		result=applyDashes(attrname,a->second.getValue(),result);
		//		applied[3]=true;
	    }
	    if (!applied[4]) {
		result=applyMusic(attrname,a->second.getValue(),result);
		//		applied[4]=true;
	    }
	}
    }
    return result;
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



