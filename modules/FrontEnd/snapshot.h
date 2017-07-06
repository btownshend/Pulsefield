/*
 * snapshot.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once

#include <vector>

class SickIO;
class World;

class Snapshot {
#ifdef MATLAB
    std::vector<mxArray *>  vis;
    std::vector<mxArray *>  bg;
    std::vector<mxArray *> world;
#endif
    std::vector<std::string> arglist;
public:
    Snapshot(const std::vector<std::string> &arglist);
    void append(int frame, const Vis *vis, const World *world);
    void save(const char *filename) const;
    void clear();
#ifdef MATLAB
    int size() { return vis.size(); }
#endif
};
