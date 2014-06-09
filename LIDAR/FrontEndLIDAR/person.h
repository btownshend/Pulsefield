/*
 * person.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once

#include <ostream>
#include <mat.h>
#include "lo/lo.h"
#include "point.h"
#include "legstats.h"
#include "leg.h"

class Vis;
class Group;

class Person {
    // Overall 
    int id;
    int channel;
    Point position;
    Point velocity;

    // Grouping
    std::shared_ptr<Group> group;   // Current group or null if ungrouped

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
    std::shared_ptr<Group> getGroup() const { return group; }
    void addToGroup(std::shared_ptr<Group> g);
    void unGroup();
    bool isGrouped() const { return group!=nullptr; }
    float getObsLike(const Point &pt, int leg, int frame) const;   // Get likelihood of an observed echo at pt hitting leg given current model
    // Send /pf/ OSC messages
    void sendMessages(lo_address &addr, int frame, double now) const;
};


class People {
    int nextid;
    std::vector <std::shared_ptr<Person> > p;
 public:
    People() { nextid=1; }
    unsigned int size() const { return p.size(); }
    const Person &operator[](int i) const { return *p[i]; }
    Person &operator[](int i)  { return *p[i]; }
    void add(const Point &l1, const Point &l2) {
	p.push_back(std::shared_ptr<Person>(new Person(nextid,l1,l2)));
	nextid++;
    }
    void erase(int i) { p.erase(p.begin()+i); }
    std::vector <std::shared_ptr<Person> >::iterator begin() { return p.begin(); }
    std::vector <std::shared_ptr<Person> >::iterator end() { return p.end(); }
    std::vector <std::shared_ptr<Person> >::const_iterator begin() const { return p.begin(); }
    std::vector <std::shared_ptr<Person> >::const_iterator end() const { return p.end(); }
};
