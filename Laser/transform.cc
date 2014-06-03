#include <iostream>
#include <iomanip>
#include "opencv2/imgproc/imgproc.hpp"

#include "transform.h"
#include "drawing.h"

Transform::Transform(): floorpts(4), devpts(4) {
    clear();
}

void Transform::clear() {
    // Default mapping
    // Point indices are order based on laser positions: BL, BR, TR, TL
    floorpts[0]=cv::Point2f(-3,3);
    devpts[0]=cv::Point2f(-32767,32768);
    floorpts[1]=cv::Point2f(3,3);
    devpts[1]=cv::Point2f(32767,32768);
    floorpts[2]=cv::Point2f(3,0);
    devpts[2]=cv::Point2f(32767,-32768);
    floorpts[3]=cv::Point2f(-3,0);
    devpts[3]=cv::Point2f(-32767,-32768);

    recompute();
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
	transform=cv::getPerspectiveTransform(floorpts,devpts);
	invTransform=cv::getPerspectiveTransform(devpts,floorpts);
    }
    dbg("Transform.recompute",1) << "Done" << std::endl;
}

etherdream_point Transform::mapToDevice(CPoint floorPt) const {
    etherdream_point p;
    std::vector<cv::Point2f> src(1);
    src[0].x=floorPt.X();
    src[0].y=floorPt.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,transform);
    int x=round(dst[0].x);
    if (x<-32768)
	p.x=-32768;
    else if (x>32767)
	p.x=32767;
    else
	p.x=x;

    int y=round(dst[0].y);
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

    dbg("Transform.mapToDevice",10) << floorPt << " -> " << "[" << p.x << "," <<p.y << "]" << std::endl;
    return p;
}

Point Transform::mapToDevice(Point floorPt) const {
    Point p;
    std::vector<cv::Point2f> src(1);
    src[0].x=floorPt.X();
    src[0].y=floorPt.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,transform);
    return Point(dst[0].x,dst[0].y);
}

CPoint Transform::mapToWorld(etherdream_point p) const {
    std::vector<cv::Point2f> src(1);
    src[0].x=p.x;
    src[0].y=p.y;
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,invTransform);
    CPoint result(dst[0].x,dst[0].y,Color(p.r/65535.0,p.g/65535.0,p.b/65535.0));

    dbg("Transform.mapToWorld",10)  << "[" << p.x << "," <<p.y << "]  -> " << result << std::endl;
    return result;
}

Point Transform::mapToWorld(Point p) const {
    std::vector<cv::Point2f> src(1);
    src[0].x=p.X();
    src[0].y=p.Y();
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,invTransform);
    Point result(dst[0].x,dst[0].y);

    dbg("Transform.mapToWorld",10)  << "[" << p.X() << "," <<p.Y() << "]  -> " << result << std::endl;
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
	s << std::fixed <<  std::setprecision(3) << floorpts[i].x << " " << floorpts[i].y << " " << std::setprecision(0) << devpts[i].x << " " << devpts[i].y << " " << std::endl << std::setprecision(3);;
}

void Transform::load(std::istream &s) {
    dbg("Transform.load",1) << "Loading transform" << std::endl;
    for (unsigned int i=0;i<floorpts.size();i++) 
	s >> floorpts[i].x >> floorpts[i].y  >> devpts[i].x >> devpts[i].y;
    recompute();
}

