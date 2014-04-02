/*
 * world.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <ostream>
#include "classifier.h"
#include "target.h"
#include "person.h"
#include "dest.h"

class Vis;

class World {
    int lastframe;
    int nextid;
    std::vector<Person> people;
    std::set<int> lastid;
    struct timeval starttime;
public:
    World();
    // Track people and send update messages
    void track(const Targets &targets, const Vis &vis, int frame, float fps);
    void deleteLostPeople();
    void sendMessages(const Destinations &dests, const struct timeval &acquired);
    mxArray *convertToMX() const;
};

#endif  /* WORLD_H_ */
