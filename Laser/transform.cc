#include <iostream>
#include <iomanip>
#include "opencv2/imgproc/imgproc.hpp"

#include "transform.h"
#include "drawing.h"

Transform::Transform(): floorpts(4), devpts(4) {
    hfov=90*M_PI/180;
    vfov=90*M_PI/180;
    clear();
}

void Transform::clear() {
    // Default mapping
    // Point indices are order based on laser positions: BL, BR, TR, TL
    floorpts[0]=Point(-3,3);
    devpts[0]=Point(-32767,32768);
    floorpts[1]=Point(3,3);
    devpts[1]=Point(32767,32768);
    floorpts[2]=Point(3,0);
    devpts[2]=Point(32767,-32768);
    floorpts[3]=Point(-3,0);
    devpts[3]=Point(-32767,-32768);

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
    if (floorpts.size() != 4) {
	dbg("Transform.recompute",0) << "recompute() called after " <<floorpts.size() << " points added -- must be exactly 4" << std::endl;
    } else {
	transform=cv::getPerspectiveTransform(convertPoints(floorpts),convertPoints(deviceToFlat(devpts)));
	invTransform=cv::getPerspectiveTransform(convertPoints(deviceToFlat(devpts)),convertPoints(floorpts));
    }
    dbg("Transform.recompute",1) << "Done" << std::endl;
}

// Convert  device coord to flat space (i.e. laser projection grid)
Point Transform::deviceToFlat(Point devPt) const {
    float phi=devPt.X()*hfov/65535;
    float theta=devPt.Y()*vfov/65535;
    Point flatPt(phi/cos(theta)/cos(phi)*2.0/hfov, theta/cos(theta)*2.0/vfov);
    dbg("Transform.deviceToFlat",10) << devPt << " -> phi = " <<  phi << ", theta=" << theta << " -> " << flatPt << std::endl;
    return flatPt;
}

// Solve y=theta/cos(theta) for theta
static float unmap(float y) {
    static const float TOL=.00001;
    int i;
    float theta1=-M_PI/2, theta2=M_PI/2;
    float theta;
    for (i=0;i<100;i++) {	// Iterate to find final solution
	theta=(theta1+theta2)/2;
	float yhat=theta/cos(theta);
	dbg("Transform.umap",11) << "i=" << i << ", theta=" <<theta << ", yhat=" << yhat << std::endl;
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
    return theta;
}

// Convert  flat space coord to device coords
Point Transform::flatToDevice(Point flatPt) const {
    float theta=unmap(flatPt.Y()*vfov/2);
    float phi=unmap(flatPt.X()*cos(theta)*hfov/2);
    Point devPt(phi*65535/hfov, theta*65535/vfov);
    dbg("Transform.flatToDevice",10) << flatPt << " -> phi = " <<  phi << ", theta=" << theta << " -> " << devPt << std::endl;
    return devPt;
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

etherdream_point Transform::mapToDevice(CPoint floorPt) const {
    Point devPt=mapToDevice((Point)floorPt);
    etherdream_point p;
    int x=round(devPt.X());
    if (x<-32768)
	p.x=-32768;
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

    Color c=floorPt.getColor();
    p.r=(int)(c.red() * 65535);
    p.g=(int)(c.green() * 65535);
    p.b=(int)(c.blue() * 65535);

    dbg("Transform.mapToDevice(CP)",10) << floorPt << " -> " << "[" << p.x << "," <<p.y << "]" << std::endl;
    return p;
}

Point Transform::mapToDevice(Point floorPt) const {
    Point p;
    std::vector<cv::Point2f> src(1);
    src[0].x=floorPt.X();
    src[0].y=floorPt.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,transform);
    // dst is now in 'flat' space
    Point devPt=flatToDevice(Point(dst[0].x, dst[0].y));
    dbg("Transform.mapToDevice",10) << floorPt << " -> " << "[" << dst[0].x << "," << dst[0].y << "] -> " << devPt << std::endl;
    return devPt;
}

CPoint Transform::mapToWorld(etherdream_point p) const {
    Point floorPt=mapToWorld(Point(p.x,p.y));
    CPoint result(floorPt,Color(p.r/65535.0,p.g/65535.0,p.b/65535.0));

    dbg("Transform.mapToWorld(EP)",10)  << "[" << p.x << "," <<p.y << "]  -> " << result << std::endl;
    return result;
}

Point Transform::mapToWorld(Point devPt) const {
    std::vector<cv::Point2f> src(1);
    Point flatPt=deviceToFlat(devPt);
    src[0].x=flatPt.X();
    src[0].y=flatPt.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,invTransform);
    Point result(dst[0].x,dst[0].y);

    dbg("Transform.mapToWorld",10)  <<  devPt << "  -> " << result << std::endl;
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

