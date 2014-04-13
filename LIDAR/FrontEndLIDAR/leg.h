#ifndef LEG_H_
#define LEG_H_

#include <ostream>
#include <vector>
#include "point.h"

class Person;
class LegStats;
class Vis;

class Leg {
    friend Person;
    Point position;
    float posvar;
    Point prevPosition;
    float prevposvar;
    Point velocity;
    std::vector<int> scanpts;
    float maxlike; 	   // Likelihood of maximum likelihood estimator
    int consecutiveInvisibleCount;
    // Keep the likelihood map so we can dump to matlab
    std::vector<float> like;
    int likenx, likeny;
    Point minval, maxval;
    void init(const Point &pt);
 public:
    Leg();
    Leg(const Point &pos);
    friend std::ostream &operator<<(std::ostream &s, const Leg &l);
    float getObsLike(const Point &pt, int frame,const LegStats &ls) const;
    Point getPosition() const { return position; }
    void predict(int nstep, float fps);
    void update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs, int nstep,float fps, const LegStats &ls, const Leg *otherLeg=0);
    void sendMessages(lo_address &addr, int frame, int id, int legnum) const;
};

#endif /* LEG_H_ */
