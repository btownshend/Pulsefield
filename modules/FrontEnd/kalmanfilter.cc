#include "kalmanfilter.h"
#include "dbg.h"

void KalmanFilter::init(Point init) {
    int dynamParams=4;
    int measureParams=2;
    kf.init(dynamParams,measureParams,0,CV_32F);

    // Transition matrix - constant velocity model
    kf.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    cv::Mat_<float> measurement(2,1);
    measurement.setTo(cv::Scalar(0));
 
    // initial state
    kf.statePre.at<float>(0) = init.X();
    kf.statePre.at<float>(1) = init.Y();
    kf.statePre.at<float>(2) = 0;
    kf.statePre.at<float>(3) = 0;

    setIdentity(kf.measurementMatrix);
    setIdentity(kf.processNoiseCov, cv::Scalar::all(5*5));	// Process noise (prediction error) is about 5mm
    // Initial state variance  (5mm for position, 1m/s for velocity)
    setIdentity(kf.errorCovPost, cv::Scalar::all(0));
    kf.errorCovPost.at<float>(0,0)=5*5;
    kf.errorCovPost.at<float>(1,1)=5*5;
    kf.errorCovPost.at<float>(2,2)=(1000/50*1000/50);
    kf.errorCovPost.at<float>(3,3)=(1000/50*1000/50);
    dbg("KalmanFilter.init",1) << "New KF: " << *this << std::endl;
    predict();
    dbg("KalmanFilter.init",1) << "New KF after predict: " << *this << std::endl;
}


Point  KalmanFilter::correct(const Point  &meas,const Point &measvar) {
    dbg("KalmanFilter.correct",1) << "Before correct KF: " << *this << std::endl;
    cv::setIdentity(kf.measurementNoiseCov,cv::Scalar(measvar.X(),measvar.Y()));
    cv::Vec2f in(meas.X(),meas.Y());
    cv::Mat m=kf.correct((cv::Mat)in);
    dbg("KalmanFilter.correct",1) << "After correct KF: " << *this << std::endl;
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

Point KalmanFilter::getPositionVar() const {
    return Point(kf.errorCovPost.at<float>(0,0),kf.errorCovPost.at<float>(1,1));
}

std::ostream& operator<<(std::ostream &s, const KalmanFilter &k) {
    s << "State= " << k.kf.statePost << " ";
    s << "ErrorCovPost=" << k.kf.errorCovPost << " ";
    s << "ErrorCovPre=" << k.kf.errorCovPre << " ";
    return s;
}
