#pragma once

#include <vector>
#include <string>
#include <pthread.h>
#include "lo_util.h"

class OSCHandler {
    int serverPort;

    lo_server s;
    
    pthread_t incomingThread;
    void processIncoming();
    static void *processIncoming(void *arg);  // Static version of pthread_create

    float minx,maxx,miny,maxy;
    void updateBounds();
    int lastUpdateFrame;   // Frame of last /laser/update message received
    struct timeval lastFrameTime;	// Time of receipt of last frame message
 public:
    OSCHandler(int port);
    ~OSCHandler();

    void run();
    void wait() {
	// Wait till done
	pthread_join(incomingThread,NULL);
    }
    // Handlers for OSC messages
    void startStop(bool start);
    void ping(lo_message msg, int seqnum);


    void pfframe(int frame, bool fake=false);

    // Changing bounds
    void setMinX(float x) { minx=x; updateBounds(); }
    void setMaxX(float x) { maxx=x; updateBounds(); }
    void setMinY(float y) { miny=y; updateBounds(); }
    void setMaxY(float y) { maxy=y; updateBounds(); }
};
