/*
 * world.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "classifier.h"
#include "target.h"
#include "person.h"

class Vis;

class World {
    int lastframe;
    int nextid;
    std::vector<Person> people;
public:
    World();
    // Track people and send update messages
    void track(const Targets &targets, const Vis &vis, int frame, float fps);
    void deleteLostPeople();
    mxArray *convertToMX() const;
    void print() const;
};

#endif  /* WORLD_H_ */
