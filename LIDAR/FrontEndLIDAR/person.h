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
class Likelihood;

class Person {
    // Overall 
    int id;
    Point position;
    Point velocity;
    int age;
    int consecutiveInvisibleCount;
    int totalVisibleCount;

    // Leg positions, etc
    Point legs[2];
    Point prevlegs[2];
    Point legvelocity[2];
    int  legclasses[2];
    float posvar[2];
    float legdiam;
    float leftness;

    Point circmodel(const Target *t, bool update);
    Point nearestShadowed(const Vis &vis,Point otherlegpos,Point targetpos);
public:
    Person(int _id, const Vis &vis, const Target *t1, const Target *t2);
    void getclasslike(const Targets &targets, const Vis &vis, Likelihood &likes, int tracknum);
    static void newclasslike(const Targets &targets, const Vis &vis, Likelihood &likes);
    void predict(int nstep, float fps);
    void update(const Vis &vis, const Target *t1, const Target *t2, int nstep,float fps);
    void addToMX(mxArray *people, int index) const;
    friend std::ostream &operator<<(std::ostream &s, const Person &p);
    bool isDead() const;
};

#endif  /* PERSON_H_ */
 
