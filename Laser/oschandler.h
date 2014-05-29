#pragma once

#include <vector>
#include <string>
#include <pthread.h>
#include "lo/lo.h"
#include "drawing.h"
#include "lasers.h"

class Video;

class OSCHandler {
    Drawing drawing;
    std::shared_ptr<Lasers> lasers;
    std::shared_ptr<Video> video;
    int serverPort;

    int unit;
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
    void setPPS(int unit,int pps);
    void setPoints(int unit,int points);	// Target points per frame
    void setBlanking(int unit,int before, int after);
    void setSkew(int unit, int s);

    // Attributes
    void setColor(Color c);
    void setDensity(float d);
    void setAttribute(const char *attr, float value);

    // Primitives
    void shapeBegin(const char *type);
    void shapeEnd();
    void circle(Point center, float radius);
    void arc(Point center, Point perim,  float angleCW);
    void line(Point p1, Point p2);
    void cubic(Point p1, Point p2, Point p3, Point p4);

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
};
