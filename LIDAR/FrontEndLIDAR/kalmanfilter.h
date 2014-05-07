#pragma once

#include <video/tracking.hpp>
#include "point.h"

// Wrapper around openCV Kalman filter class for doing constant-velocity tracking in two dimensions with different measurement variances for each frame
class KalmanFilter {
    cv::KalmanFilter kf;
 public:
    KalmanFilter(): kf() { init(Point(0,0)); }
    KalmanFilter(const Point &pt): kf() { init(pt); }
    void init(Point init);
    Point predict() {
	cv::Mat m=kf.predict();
	return Point(m.at<float>(0),m.at<float>(1));
    }
    Point  correct(const Point  &meas,const Point &measvar);
    Point getVelocity(float fps) const;
    void setVelocity(float fps,Point vel);
    Point getPosition() const;
};

 
