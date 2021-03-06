#pragma once
#include <ostream>
#include <set>
#include <vector>
#include <assert.h>
#include "lo/lo.h"
#include "point.h"

class People;

class Group {
    int gid;
    std::set<int> members;
    double createTime;
    Point centroid;
    float diameter;
 public:
    Group(int _gid, double now) { gid=_gid; createTime=now; diameter=0; }
    ~Group();
    void add(int uid) { assert(members.count(uid)==0);  members.insert(uid); }
    void remove(int uid);
    int size() const { return members.size(); }
    void update(const People &people);
    int getID() const { return gid; }
   void sendMessages(lo_address &addr, int frame, double now) const;
   void setCentroid(Point p) { centroid=p; }
   Point getCentroid() const { return centroid; }
   void setDiameter(float diam) { diameter=diam; }
   float getDiameter() const { return diameter; } 
    friend std::ostream &operator<<(std::ostream &s, const Group &g);
};

class Groups {
    const float groupDist, unGroupDist;
    std::set<std::shared_ptr<Group> > groups;
    std::set<int> getConnected(int i, std::set<int> current,const People &people);
    int nextID;
    std::shared_ptr<Group> newGroup(double elapsed);
 public:
    Groups(float _groupDist, float _unGroupDist): groupDist(_groupDist),unGroupDist(_unGroupDist) { nextID=1; }
    void update(People &people, double now);
    void sendMessages(lo_address &addr, int frame, double now) const;
    unsigned int size() const { return groups.size(); }
};
