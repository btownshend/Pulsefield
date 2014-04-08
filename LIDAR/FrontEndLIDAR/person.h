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
#include "point.h"
#include "target.h"

class Vis;

class Person {
    // Overall 
    int id;
    int channel;
    Point position;
    Point velocity;
    int age;
    int consecutiveInvisibleCount;
    int totalVisibleCount;

    // Leg positions, etc
    Point legs[2];
    Point prevlegs[2];
    Point legvelocity[2];
    std::vector<int> scanpts[2];
    float posvar[2];
    float prevposvar[2];
    float legdiam;
    float leftness;
    float maxlike; 	   // Likelihood of maximum likelihood estimator
    // Keep the likelihood map so we can dump to matlab
    std::vector<float> like[2];
    int likenx, likeny;
    Point minval, maxval;

    void init(int _id, const Point &leg1, const Point &leg2);
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
    const Point* getLegs() const { return legs; }
    float getLegDiam() const { return legdiam; }
    int getAge() const { return age; }
    float getObsLike(const Point &pt, int leg, int frame) const;   // Get likelihood of an observed echo at pt hitting leg given current model
};

#endif  /* PERSON_H_ */
 
