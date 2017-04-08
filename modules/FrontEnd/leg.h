#pragma once

#include <ostream>
#include <vector>
#include "point.h"

class Person;
class LegStats;
class Vis;

class Leg {
    friend class Person;
    Point position;   // Estimate of position of leg
    Point predictedPosition;  // Current predicted position before incorporating measurements
    float posvar;
    float prevposvar;
    std::vector<Point> priorPositions;	// Last n priorPositions (interpolated if we advanced multiple steps)
    Point velocity;	// Velocity for publishing to OSC; not for kinematics
    std::vector<int> scanpts;
    float maxlike; 	   // Likelihood of maximum likelihood estimator
    int consecutiveInvisibleCount;
    // Keep the likelihood map so we can dump to matlab
    std::vector<float> like;
    int likenx, likeny;
    Point minval, maxval;
    void init(const Point &pt);
    // Weights for predicting next delta
    std::vector<float> predictWeights;   // interleaved:   this leg[-1],otherleg[-1],thisleg[-2],...
 public:
    Leg();
    Leg(const Point &pos);
    friend std::ostream &operator<<(std::ostream &s, const Leg &l);
    float getObsLike(const Point &pt, int frame,const LegStats &ls) const;
    Point getPosition() const { return position; }
    void savePriorPositions();
    // Get prior position from n frames ago (n>0)
    Point getPriorPosition(int n) const;
    // Get delta from n frames ago (n>0)
    Point getPriorDelta(int n) const;
    Point getVelocity() const { return velocity; }
    void predict(const Leg &otherLeg);
    void update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs, const LegStats &ls, const Leg *otherLeg=0);
    void updateVisibility(const std::vector<float> &bglike);
    void updateVelocity(int nstep, float fps,Point otherLegVelocity);
    void updateDiameterEstimates(const Vis &vis, LegStats &ls) const;   // Update given legstats diameter if possible
    void sendMessages(lo_address &addr, int frame, int id, int legnum) const;
    bool isVisible() const { return consecutiveInvisibleCount==0; }
    void setupGrid(int _likenx, int _likeny, Point _minval, Point _maxval) { likenx=_likenx; likeny=_likeny; minval=_minval; maxval=_maxval; }
    float getFramePerformance() const { return pow((position-predictedPosition).norm(),2.0); }
};