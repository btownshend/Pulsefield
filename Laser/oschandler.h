#pragma once

#include <vector>
#include <string>
#include <pthread.h>
#include "lo/lo.h"
#include "dest.h"
#include "drawing.h"

class Laser;

class OSCHandler {
    Drawing drawing;
    Laser *laser;
    int serverPort;

    int unit;
    Destinations dests;
    lo_server s;
    
    pthread_t incomingThread;
    static void *processIncoming(void *arg);

 public:
    OSCHandler(int unit, Laser *_laser);
    ~OSCHandler();

    void run();
    void wait() {
	// Wait till done
	pthread_join(incomingThread,NULL);
    }
    // Handlers for OSC messages
    void startStop(bool start);
    void setFPS(int fps);
    int getFPS() const;
    void ping(lo_message msg, int seqnum);
    void circle(lo_message msg, float x, float y, float radius, float r, float g, float b);
    void line(lo_message msg, float x1, float y1, float x2, float y2, float r, float g, float b);
    void update(lo_message msg, int nPoints);

    // Destination handling
    void addDest(const char *host, int port);
    void addDest(lo_message msg, int port);
    void rmDest(const char *host, int port);
    void rmDest(lo_message msg, int port);
    void rmAllDest();
};
