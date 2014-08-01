#pragma once

#include <vector>
#include <string>
#include <pthread.h>
#include "lo/lo.h"
#include "drawing.h"
#include "lasers.h"

class Video;

class OSCHandler {
    Drawing bgDrawing,cellDrawing,conxDrawing;
    enum { NONE,BACKGROUND,CELL,CONX} drawTarget;
    std::shared_ptr<Lasers> lasers;
    std::shared_ptr<Video> video;
    int serverPort;

    lo_server s;
    
    pthread_t incomingThread;
    void processIncoming();
    static void *processIncoming(void *arg);  // Static version of pthread_create

    Color currentColor;
    Attributes currentAttributes;
    float currentDensity;

    float minx,maxx,miny,maxy;
    void updateBounds();

    bool dirty;

    int lastUpdateFrame;   // Frame of last /laser/update message received
    struct timeval lastFrameTime;	// Time of receipt of last frame message
    Drawing *currentDrawing();   // Get currently targetted drawing or NULL if none.
 public:
    OSCHandler(int port, std::shared_ptr<Lasers> lasers, std::shared_ptr<Video> video);
    ~OSCHandler();

    void run();
    void wait() {
	// Wait till done
	pthread_join(incomingThread,NULL);
    }
    // Handlers for OSC messages
    void startStop(bool start);
    void ping(lo_message msg, int seqnum);

    // Laser settings
    void setPPS(int pps);
    void setPoints(int points);	// Target points per frame
    void setPreBlanking(int n);
    void setPostBlanking(int n);
    void setSkew( int s);

    // Attributes
    void setColor(Color c);
    void setDensity(float d);

    // Primitives
    void cellBegin(int uid);
    void cellEnd(int uid);
    void conxBegin(const char *cid);
    void conxEnd(const char *cid);
    void bgBegin();
    void bgEnd();
    void circle(Point center, float radius);
    void arc(Point center, Point perim,  float angleCW);
    void line(Point p1, Point p2);
    void cubic(Point p1, Point p2, Point p3, Point p4);
    void svgfile(std::string filename,Point origin, float scaling, float rotateDeg);

    void update(int frame);
    void map(int unit, int pt, Point world, Point local);
    //    void setTransform(int unit);

    void pfframe(int frame);
    void pfbody(Point pos);
    void pfleg(Point pos);
    void pfbackground(int scanpt, int totalpts, float angleDeg, float range) { 
	lasers->setBackground(scanpt,totalpts,angleDeg,range);
    }

    // Changing bounds
    void setMinX(float x) { minx=x; updateBounds(); }
    void setMaxX(float x) { maxx=x; updateBounds(); }
    void setMinY(float y) { miny=y; updateBounds(); }
    void setMaxY(float y) { maxy=y; updateBounds(); }

    // Sound
    void beat(int bar, int beat);
    void tempo(float bpm);
};
