#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"

#include "transform.h"
#include "drawing.h"

Transform::Transform() {
    clear();
}

void Transform::clear() {
    transform=cv::Mat::eye(3,3,CV_32F);
    invTransform=cv::Mat::eye(3,3,CV_32F);
}

// Find the perspective transform that best matches the 3 points loaded
void Transform::setTransform() {
    dbg("Transform.setTransform",1) << "Set transform using mapping: " << std::endl;
    for (unsigned int i=0;i<floorpts.size();i++) {
	dbg("Transform.setTransform",1) << "W@" << floorpts[i] << "->D@" << devpts[i] << std::endl;
    }
    if (floorpts.size() != 4) {
	std::cerr << "setTransform() called after " <<floorpts.size() << " points added -- must be exactly 4" << std::endl;
    } else {
	transform=cv::getPerspectiveTransform(floorpts,devpts);
	invTransform=cv::getPerspectiveTransform(devpts,floorpts);
    }
    floorpts.clear();
    devpts.clear();
}

etherdream_point Transform::mapToDevice(Point floorPt,const Color &c) const {
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

Point Transform::mapToWorld(etherdream_point p) const {
    std::vector<cv::Point2f> src(1);
    src[0].x=p.x;
    src[0].y=p.y;
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src,dst,invTransform);
    Point result(dst[0].x,dst[0].y);

    dbg("Transform.mapToWorld",10)  << "[" << p.x << "," <<p.y << "]  -> " << result << std::endl;
    return result;
}

std::vector<etherdream_point> Transform::mapToDevice(const std::vector<Point> &floorPts,Color c) const {
    std::vector<etherdream_point> result(floorPts.size());
    for (unsigned int i=0;i<floorPts.size();i++)
	result[i]=mapToDevice(floorPts[i],c);
    return result;
}

std::vector<Point> Transform::mapToDevice(const std::vector<Point> &floorPts) const {
    std::vector<Point> result(floorPts.size());
    for (unsigned int i=0;i<floorPts.size();i++)
	result[i]=mapToDevice(floorPts[i]);
    return result;
}

std::vector<Point> Transform::mapToWorld(const std::vector<etherdream_point> &pts) const {
    std::vector<Point> result(pts.size());
    for (unsigned int i=0;i<pts.size();i++)
	result[i]=mapToWorld(pts[i]);
    return result;
}

