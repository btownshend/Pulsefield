#pragma once

#include <video/tracking.hpp>
#include "point.h"

// Wrapper around openCV Kalman filter class for doing constant-velocity tracking in two dimensions with different measurement variances for each frame
class KalmanFilter {
    cv::KalmanFilter kf;
 public:
    KalmanFilter(int dynamparams, in measureParams): kf(dynamParams,measureParams,0,CV_32F) {;}
    void init(int dynamparams, in measureParams) { kf.init(dynamParams,measureParams,0,CV_32F); }
    Point predict() {
	Mat m=kf.predict();
	return Point(m.at<float>(0),m.at<float>(1));
    }
    Point  correct(const Point  &meas,const Point &measSEM) {
	// TODO - update measurevar 
	cv::setIdentity(kf.measurementNoiseCov,Scalar::all(measSEM));
	Vec2f in(meas.X(),meas.Y());
	Mat m=kf.correct(in);
	return Point(m.at<float>(0),m.at<float>(1));
    }
}

 
