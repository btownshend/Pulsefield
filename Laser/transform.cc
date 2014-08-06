#include <iostream>
#include <iomanip>
#include "opencv2/imgproc/imgproc.hpp"

#include "transform.h"
#include "drawing.h"

Transform::Transform(): floorpts(4), devpts(4) {
    hfov=90*M_PI/180;
    vfov=90*M_PI/180;
    minx=-.96f;
    maxx=.96f;
    miny=-.76f;  // Needs to be less than extreme values in etherdream (short) values 
    maxy=.96f;
    clear(Bounds(-6,0,6,6));

    if (false) {
	// Test conversions
	Point devPt=Point(0,0);
	Point flatPt=deviceToFlat(devPt);
	Point dev2=flatToDevice(flatPt);
	dbg("Transform.transform",1) << "Device Point " << devPt << " -> " << flatPt << " -> " << dev2 << std::endl;
	devPt=Point(-32767,0);
	flatPt=deviceToFlat(devPt);
	dev2=flatToDevice(flatPt);
	dbg("Transform.transform",1) << "Device Point " << devPt << " -> " << flatPt << " -> " << dev2 << std::endl;
	devPt=Point(0,32767);
	flatPt=deviceToFlat(devPt);
	dev2=flatToDevice(flatPt);
	dbg("Transform.transform",1) << "Device Point " << devPt << " -> " << flatPt << " -> " << dev2 << std::endl;

	devPt=Point(0,32767);
	flatPt=deviceToFlat(devPt);
	dev2=flatToDevice(flatPt);
	dbg("Transform.transform",1) << "Device Point " << devPt << " -> " << flatPt << " -> " << dev2 << std::endl;


	devPt=Point(0,32767*10);
	Point floorPt=mapToWorld(devPt);
	dev2=mapToDevice(floorPt);
	dbg("Transform.transform",1) << "Device Point " << devPt << " -> Floor: " << floorPt << " -> " << dev2 << std::endl;

	flatPt=Point(-3,4);
	devPt=flatToDevice(flatPt);
	Point flat2=deviceToFlat(devPt);
	dbg("Transform.transform",1) << "Flat Point " << flatPt << " -> " << devPt << " -> " << flat2 << std::endl;

	flatPt=Point(10,10);
	devPt=flatToDevice(flatPt);
	flat2=deviceToFlat(devPt);
	dbg("Transform.transform",1) << "Flat Point " << flatPt << " -> " << devPt << " -> " << flat2 << std::endl;

	flatPt=Point(1,1);
	devPt=flatToDevice(flatPt);
	flat2=deviceToFlat(devPt);
	dbg("Transform.transform",1) << "Flat Point " << flatPt << " -> " << devPt << " -> " << flat2 << std::endl;

	floorPt=Point(0,0);
	devPt=mapToDevice(floorPt);
	Point floor2=mapToWorld(devPt);
	dbg("Transform.transform",1) << "Floor Point " << floorPt << " -> " << devPt << " -> " << floor2 << std::endl;

	floorPt=Point(-1,0);
	devPt=mapToDevice(floorPt);
	floor2=mapToWorld(devPt);
	dbg("Transform.transform",1) << "Floor Point " << floorPt << " -> " << devPt << " -> " << floor2 << std::endl;

	floorPt=Point(0,1);
	devPt=mapToDevice(floorPt);
	floor2=mapToWorld(devPt);
	dbg("Transform.transform",1) << "Floor Point " << floorPt << " -> " << devPt << " -> " << floor2 << std::endl;

	floorPt=Point(3,1);
	devPt=mapToDevice(floorPt);
	floor2=mapToWorld(devPt);
	dbg("Transform.transform",1) << "Floor Point " << floorPt << " -> " << devPt << " -> " << floor2 << std::endl;
    }
}

void Transform::clear(const Bounds &floorBounds) {
    // Default mapping
    // Point indices are order based on laser positions: BL, BR, TR, TL
    floorpts[0]=Point(floorBounds.getMinX(),floorBounds.getMaxY());
    devpts[0]=flatToDevice(Point(minx,maxy));
    floorpts[1]=Point(floorBounds.getMaxX(),floorBounds.getMaxY());
    devpts[1]=flatToDevice(Point(maxx,maxy));
    floorpts[2]=Point(floorBounds.getMaxX(),floorBounds.getMinY());
    devpts[2]=flatToDevice(Point(maxx,miny));
    floorpts[3]=Point(floorBounds.getMinX(),floorBounds.getMinY());
    devpts[3]=flatToDevice(Point(minx,miny));

    recompute();
}

std::vector<cv::Point2f> Transform::convertPoints(const std::vector<Point> &pts) {
    std::vector<cv::Point2f> result(pts.size());
    for (int i=0;i<pts.size();i++) {
	result[i].x=pts[i].X();
	result[i].y=pts[i].Y();
    }
    return result;
}

// Find the perspective transform that best matches the 3 points loaded
void Transform::recompute() {
    dbg("Transform.recompute",1) << "Set transform using mapping: " << std::endl;
    for (unsigned int i=0;i<floorpts.size();i++) {
	dbg("Transform.recompute",1) << "W@" << floorpts[i] << "->D@" << devpts[i] << std::endl;
    }
    dbg("Transform.recompute",1) << "VFOV=" << vfov << ", HFOV=" << hfov << std::endl;
    if (floorpts.size() != 4) {
	dbg("Transform.recompute",0) << "recompute() called after " <<floorpts.size() << " points added -- must be exactly 4" << std::endl;
    } else {
	transform=cv::getPerspectiveTransform(convertPoints(floorpts),convertPoints(deviceToFlat(devpts)));
	dbg("Transform.recompute",1) << "Transform=" << std::endl;
	for (int i=0;i<transform.rows;i++) {
	    for (int j=0;j<transform.cols;j++)
		dbgn("Transform.recompute",1) << transform.at<double>(i,j) << " ";
	    dbgn("Transform.recompute",1) << std::endl;
	}
		
	invTransform=cv::getPerspectiveTransform(convertPoints(deviceToFlat(devpts)),convertPoints(floorpts));
	dbg("Transform.recompute",1) << "invTransform=" << std::endl;
	for (int i=0;i<transform.rows;i++) {
	    for (int j=0;j<transform.cols;j++)
		dbgn("Transform.recompute",1) << invTransform.at<double>(i,j) << " ";
	    dbgn("Transform.recompute",1) << std::endl;
	}
    }
    // Calculate down tilt of laser
    // Point aimed=deviceToWorld(Point(0,0));
    // Point closer=deviceToWorld(Point(0,-1000));
    // Point distant=closer+(aimed-closer)*1000;  // Very far away
    // Point devDistant=worldToDevice(distant);

    // Calculate location of laser and its field of view in 3d space
    origin=flatToWorld(Point(0,-5));   // Just use the projection down in laser coord system (will be behind true origin due to tilt, height (TODO: FIX)
    if (origin.isNan())  {
	dbg("Transform.recompute",1) << "origin is undefined -- trying with laser assumed to be tilted up" << std::endl;
	origin=flatToWorld(Point(0,5));   // Just use the projection down in laser coord system (will be behind true origin due to tilt, height (TODO: FIX)
	if (origin.isNan())  {
	    dbg("Transform.recompute",1) << "origin still undefined, setting to 0,0" << std::endl;
	    origin=Point(0,0);
	}
    }
    
    dbg("Transform.recompute",1) << "origin=" << origin << std::endl;
}

// Convert  device coord to flat space (i.e. laser projection grid)
Point Transform::deviceToFlat(Point devPt) const {
    float phi=devPt.X()*hfov/65535;
    float theta=devPt.Y()*vfov/65535;
    Point flatPt(phi/cos(theta)/cos(phi)*2.0/hfov*cos(hfov/2), theta/cos(theta)*2.0/vfov*cos(vfov/2));
    dbg("Transform.deviceToFlat",10) << devPt << " -> phi = " <<  phi << ", theta=" << theta << " -> " << flatPt << std::endl;
    return flatPt;
}

// Solve y=theta/cos(theta) for theta
static float unmap(float y) {
    static std::map<int,float> cache;
    static int cnt=0,misses=0;
    
    int yi=(int)(y*10000);
    cnt++;
    if (cache.count(yi)>0)
	return cache[yi];
    misses++;
    if (misses%1000 == 0)
	std::cout << "Cache stats:  cnt=" << cnt << ", misses=" << misses << " hit rate=" << 100-misses*100/cnt << std::endl;

    static const float TOL=.00001;
    int i;
    float theta1=-M_PI/2, theta2=M_PI/2;
    float theta;
    for (i=0;i<100;i++) {	// Iterate to find final solution
	theta=(theta1+theta2)/2;
	float yhat=theta/cos(theta);
	dbg("Transform.unmap",11) << "i=" << i << ", theta=" <<theta << ", yhat=" << yhat << std::endl;
	if (yhat<y)
	    theta1=theta;
	else
	    theta2=theta;
	if (fabs(theta1-theta2)<TOL)
	    break;
    }
    if (i==100) {
	dbg("Transform.unmap",1) << "Failed to converge:  y=" << y << ", theta=" << theta << std::endl;
    }
    cache[yi]=theta;
    return theta;
}

// Convert  flat space coord to device coords
Point Transform::flatToDevice(Point flatPt) const {
    float theta=unmap(flatPt.Y()*vfov/2/cos(vfov/2));
    float phi=unmap(flatPt.X()*cos(theta)*hfov/2/cos(hfov/2));
    Point devPt(phi*65535/hfov, theta*65535/vfov);
    dbg("Transform.flatToDevice",10) << flatPt << " -> phi = " <<  phi << ", theta=" << theta << " -> " << devPt << std::endl;
    return devPt;
}

// Convert  flat space coord to device coords
etherdream_point Transform::flatToDevice(CPoint flatPt) const {
    CPoint devPt(flatToDevice((Point)flatPt),flatPt.getColor());
    return cPointToEtherdream(devPt);
}

std::vector<Point> Transform::deviceToFlat(const std::vector<Point> &devPts) const {
    std::vector<Point> result(devPts.size());
    for (unsigned int i=0;i<devPts.size();i++)
	result[i]=deviceToFlat(devPts[i]);
    return result;
}

std::vector<Point> Transform::flatToDevice(const std::vector<Point> &flatPts) const {
    std::vector<Point> result(flatPts.size());
    for (unsigned int i=0;i<flatPts.size();i++)
	result[i]=flatToDevice(flatPts[i]);
    return result;
}

etherdream_point Transform::cPointToEtherdream(CPoint devPt) const {
    etherdream_point p;
    if (devPt.isNan()) {
	p.x=-32768;
	p.y=-32768;
	dbg("Transform.cPointToEtherdream",5) << "Converted NaN point to [" << p.x << "," << p.y << "]" << std::endl;
    } else {
	int x=round(devPt.X());
	if (x<-32767)
	    p.x=-32767;
	else if (x>32767)
	    p.x=32767;
	else
	    p.x=x;

	int y=round(devPt.Y());
	if (y<-32768)
	    p.y=-32768;
	else if (y>32767)
	    p.y=32767;
	else
	    p.y=y;
    }
    Color c=devPt.getColor();
    p.r=(int)(c.red() * 65535);
    p.g=(int)(c.green() * 65535);
    p.b=(int)(c.blue() * 65535);
    return p;
}

etherdream_point Transform::mapToDevice(CPoint floorPt) const {
    CPoint devPt(mapToDevice((Point)floorPt),floorPt.getColor());
    etherdream_point p=cPointToEtherdream(devPt);
    dbg("Transform.mapToDevice(CP)",10) << floorPt << " -> " << "[" << p.x << "," <<p.y << "]" << std::endl;
    return p;
}

Point Transform::mapToDevice(Point floorPt) const {
    if (floorPt.isNan())
	return floorPt;

    Point p;
    std::vector<cv::Point2f> src(1);
    src[0].x=floorPt.X();
    src[0].y=floorPt.Y();
    std::vector<cv::Point2f> dst;
    //    dbg("Transform.mapToDevice",1) << "run persp" << std::endl;
    cv::perspectiveTransform(src,dst,transform);
    cv::Vec3f src2(floorPt.X(),floorPt.Y(),1.0);
    cv::Vec3f dst2=((cv::Matx33f)transform)*src2;
    if (dst2[2]<=0) {
	dbg("Transform.mapToDevice",5) << floorPt << " out of bound: dst=" << "[" << dst2[0] << "," << dst2[1] << "," << dst2[2] << "]" << std::endl;
	return Point(nan(""),nan(""));
    }

    Point devPt2=flatToDevice(Point(dst2[0]/dst2[2], dst2[1]/dst2[2]));
    // dst is now in 'flat' space
    Point devPt=flatToDevice(Point(dst[0].x, dst[0].y));
    dbg("Transform.mapToDevice",10) << floorPt << " -> " << "[" << dst[0].x << "," << dst[0].y << "] -> " << devPt << " (or " << devPt2 << ")" << std::endl;
    return devPt;
}

CPoint Transform::mapToWorld(etherdream_point p) const {
    Point floorPt;
    if (p.x==-32768 && p.y==-32768)
	floorPt=Point(nan(""),nan(""));
    else
	floorPt=mapToWorld(Point(p.x,p.y));

    CPoint result(floorPt,Color(p.r/65535.0,p.g/65535.0,p.b/65535.0));

    dbg("Transform.mapToWorld(EP)",10)  << "[" << p.x << "," <<p.y << "]  -> " << result << std::endl;
    return result;
}

Point Transform::mapToWorld(Point devPt) const {
    Point flatPt=deviceToFlat(devPt);
    return flatToWorld(flatPt);
}

Point Transform::flatToWorld(Point flatPt) const {
    if (flatPt.isNan())
	return flatPt;

    std::vector<cv::Point2f> src(1);
    src[0].x=flatPt.X();
    src[0].y=flatPt.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,invTransform);
    cv::Vec3f src2(flatPt.X(),flatPt.Y(),1.0);
    cv::Vec3f dst2=((cv::Matx33f)invTransform)*src2;
    if (dst2[2]<=0) {
	dbg("Transform.flatToWorld",5)  <<  flatPt << " out of bound: dst=" << "[" << dst2[0] << "," << dst2[1] << "," << dst2[2] << "]" << std::endl;
	return Point(nan(""),nan(""));
    }

    Point result(dst[0].x,dst[0].y);

    dbg("Transform.flatToWorld",10)  <<  flatPt << "  -> " << result << std::endl;
    return result;
}

std::vector<etherdream_point> Transform::mapToDevice(const std::vector<CPoint> &floorPts) const {
    std::vector<etherdream_point> result(floorPts.size());
    for (unsigned int i=0;i<floorPts.size();i++)
	result[i]=mapToDevice(floorPts[i]);
    return result;
}

std::vector<Point> Transform::mapToDevice(const std::vector<Point> &floorPts) const {
    std::vector<Point> result(floorPts.size());
    for (unsigned int i=0;i<floorPts.size();i++)
	result[i]=mapToDevice(floorPts[i]);
    return result;
}

std::vector<CPoint> Transform::mapToWorld(const std::vector<etherdream_point> &pts) const {
    std::vector<CPoint> result(pts.size());
    for (unsigned int i=0;i<pts.size();i++)
	result[i]=mapToWorld(pts[i]);
    return result;
}

// Check if a given device coordinate is "on-screen" (can be projected)
bool Transform::onScreen(Point devPt) const {
    if (devPt.isNan()) {
	dbg("Transform.onScreen",5) << "devPt is nan -> false" << std::endl;
	return false;
    }
    if  (devPt.X()<-32767 || devPt.X()>32766 || devPt.Y() <-32767 || devPt.Y() >32766) {
	dbg("Transform.onScreen",5) << "devPt " << devPt << " out of bounds -> false" << std::endl;
	// Can't move the laser to this position
	return false;
    }
    // Otherwise, check if it makes it past the window
    Point flatPt=deviceToFlat(devPt);
    if (flatPt.isNan()) {
	dbg("Transform.onScreen",5) << "devPt " << devPt << " gives NaN flat Pt -> false" << std::endl;
	return false;
    }
    return flatPt.X()>=minx && flatPt.X()<=maxx && flatPt.Y() >=miny && flatPt.Y() <= maxy;
}

void Transform::save(std::ostream &s) const {
    dbg("Transform.save",1) << "Saving transform" << std::endl;
    for (unsigned int i=0;i<floorpts.size();i++) 
	s << std::fixed <<  std::setprecision(3) << floorpts[i] << " " << std::setprecision(0) << devpts[i] << " " <<  std::endl << std::setprecision(3);
}

void Transform::load(std::istream &s) {
    dbg("Transform.load",1) << "Loading transform" << std::endl;
    for (unsigned int i=0;i<floorpts.size();i++) 
	s >> floorpts[i] >> devpts[i];
    recompute();
}

void Transform::clipLine(Point &p1, Point &p2) const {
    Point dp1=mapToDevice(p1);
    Point dp2=mapToDevice(p2);
    if (onScreen(dp1)&&onScreen(dp2))
	return;
    if (!onScreen(dp1)&&!onScreen(dp2)) {
	p2=p1;
	return;
    }
    Point good,bad;
    if (onScreen(dp1)) {
	good=p1;
	bad=p2;
    } else {
	good=p2;
	bad=p1;
    }
    // Binary search for edge with 1cm resolution
    while ((good-bad).norm() > 0.01) {
	Point mid=(good+bad)/2;
	if (onScreen(mapToDevice(mid)))
	    good=mid;
	else 
	    bad=mid;
    }
    if (onScreen(dp1))
	p2=good;
    else
	p1=good;
    dbg("Transform.clipLine",5) << "Clipped " << dp1 << "-" << dp2 << " to " <<  mapToDevice(p1) << "-" << mapToDevice(p2) << std::endl;
}
