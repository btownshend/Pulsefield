/*
 * world.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <set>
#include <vector>
#include <ostream>
#include <cairo-xlib.h>
#include "target.h"
#include "person.h"
#include "dest.h"
#include "background.h"

class Vis;
typedef struct _cairo_surface cairo_surface_t;

class World {
    int lastframe;
    int nextid;
    std::vector<Person> people;
    std::set<int> lastid;

    Background bg;		// Background model
    std::vector<float> bglike;	// Current background likelihoods (regardless of assignment)
    std::vector<int> assignments;   // Which person is assigned to each scan line -- -1 for background, -2 for new track
    std::vector<unsigned int> legassigned;  		    // For each assignment, leg number assigned (0 or 1);  0 for assignments without legs (such as bg)
    std::vector<float> bestlike;					// Best likelihood for each scan line

    Display *dpy;
    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);
    void makeAssignments(const Vis &vis, float entrylike);
public:
    World();
    // Track people and send update messages
    void track( const Vis &vis, int frame, float fps);
    void deleteLostPeople();
    void sendMessages(Destinations &dests, double now);
    mxArray *convertToMX() const;

    // Drawing routines
    void initWindow();
    void draw() const;

    const Background &getBackground() const { return bg; }
};

#endif  /* WORLD_H_ */
