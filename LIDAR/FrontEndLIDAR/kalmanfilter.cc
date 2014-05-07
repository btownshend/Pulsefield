#include "kalmanfilter.h"

void KalmanFilter::init(Point init) {
    int dynamParams=4;
    int measureParams=2;
    kf.init(dynamParams,measureParams,0,CV_32F);

    // Transition matrix - constant velocity model
    kf.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    cv::Mat_<float> measurement(2,1);
    measurement.setTo(cv::Scalar(0));
 
    // initial state
    kf.statePre.at<float>(0) = init.X();
    kf.statePre.at<float>(1) = init.Y();
    kf.statePre.at<float>(2) = 0;
    kf.statePre.at<float>(3) = 0;

    setIdentity(kf.measurementMatrix);
    setIdentity(kf.processNoiseCov, cv::Scalar::all(5*5));	// Process noise (prediction error) is about 5mm
    //    setIdentity(kf.errorCovPost, cv::Scalar::all(.1));
}


Point  KalmanFilter::correct(const Point  &meas,const Point &measvar) {
    cv::setIdentity(kf.measurementNoiseCov,cv::Scalar(measvar.X(),measvar.Y()));
    cv::Vec2f in(meas.X(),meas.Y());
    cv::Mat m=kf.correct((cv::Mat)in);
    return Point(m.at<float>(0),m.at<float>(1));
}


Point KalmanFilter::getVelocity(float fps) const {
    return Point(kf.statePost.at<float>(2),kf.statePost.at<float>(3))*fps;
}

void KalmanFilter::setVelocity(float fps,Point vel) {
    kf.statePost.at<float>(2)=vel.X()/fps;
    kf.statePost.at<float>(3)=vel.Y()/fps;
}

Point KalmanFilter::getPosition() const {
    return Point(kf.statePost.at<float>(0),kf.statePost.at<float>(1));
}
