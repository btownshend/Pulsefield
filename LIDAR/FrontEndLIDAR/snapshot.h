/*
 * snapshot.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef SNAPSHOT_H_
#define SNAPSHOT_H_

#include <vector>

class SickIO;
class World;

class Snapshot {
    std::vector<mxArray *>  vis;
    std::vector<mxArray *>  bg;
    std::vector<mxArray *> world;
public:
    Snapshot();
    void append(const Vis *vis, const World *world);
    void save(const char *filename) const;
};

#endif  /* SNAPSHOT_H_ */
