/*
 * world.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once

#include <set>
#include <vector>
#include <ostream>
#include <cairo-xlib.h>
#include "person.h"
#include "dest.h"
#include "background.h"
#include "groups.h"

class Vis;
typedef struct _cairo_surface cairo_surface_t;

class World {
    int lastframe;
    People people;
    Groups groups;
    int priorngroups;
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
    bool drawRange;   // True to draw ranges on plots

    void initWindow();
    void drawinfo(cairo_t *cr, float left,  float top, float width, float height) const;

    pthread_mutex_t displayMutex;   // Mutex to prevent X11 events from drawing at the same time as main thread
public:
    World();
    // Track people and send update messages
    void track( const Vis &vis, int frame, float fps,double elapsed);
    void deleteLostPeople();
    void sendMessages(Destinations &dests, double elapsed);
    mxArray *convertToMX() const;

    // Drawing routines
    void draw(const Vis *vis=NULL) const;

    const Background &getBackground() const { return bg; }
};
