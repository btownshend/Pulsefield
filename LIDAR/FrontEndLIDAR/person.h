/*
 * person.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef PERSON_H_
#define PERSON_H_

#include <ostream>
#include <mat.h>
#include "lo/lo.h"
#include "point.h"
#include "target.h"
#include "legstats.h"
#include "leg.h"

class Vis;

class Person {
    // Overall 
    int id;
    int channel;
    Point position;
    Point velocity;

    // Aging, visibility
    int age;
    int consecutiveInvisibleCount;
    int totalVisibleCount;

    // Leg positions, stats
    Leg legs[2];
    LegStats legStats;
public:
    Person(int _id, const Point &leg1, const Point &leg2);
    ~Person();
    void predict(int nstep, float fps);
    void update(const Vis &vis, const std::vector<float> &bglike, const std::vector<int> fs[2], int nstep,float fps);
    void addToMX(mxArray *people, int index) const;
    friend std::ostream &operator<<(std::ostream &s, const Person &p);
    bool isDead() const;
    int getID() const { return id; }
    int getChannel() const { return channel; }
    Point getPosition() const { return position; }
    Point getVelocity() const { return velocity; }
    const Leg &getLeg(int i) const { return legs[i]; }
    float getMaxLike() const { return legs[0].maxlike+legs[1].maxlike; }
    const LegStats &getLegStats() const { return legStats; }
    int getAge() const { return age; }
    float getObsLike(const Point &pt, int leg, int frame) const;   // Get likelihood of an observed echo at pt hitting leg given current model
    // Send /pf/ OSC messages
    void sendMessages(lo_address &addr, int frame) const;
};

#endif  /* PERSON_H_ */
 
