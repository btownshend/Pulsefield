#ifndef _FRONTEND_H_
#define _FRONTEND_H_

#include "lo/lo.h"
#include "defs.h"
#include "dest.h"

class CamIO;
class Visible;

class FrontEnd {
    static const int serverPort;

    int ncamera, nled;
    CamIO **cameras;
    Visible **vis;
    Destinations dests;
    lo_server s;
    
    int frame;
    long int tosend;

    void processFrames();
    void sendMessages();   // Send messages to OSC destinations

 public:
    enum { CORR=1, VIS=2, REFIMAGE=4,IMAGE=8 };  // Bitmasks of what to send in next message group

    FrontEnd(int ncamera, int nled);
    void run();

    // Handlers for OSC messages
    void startStop(bool start);
    void setPos(int camera, int led, int xpos, int ypos, int tgtWidth, int tgtHeight);
    void setFPS(int fps);
    void setUpdateTC(float updateTime);
    void setCorrThresh(float thresh);
    void setRes(int camera,const char *res);
    void setROI(int camera, int x0, int x1, int y0, int y1);
    void setRefImage(int camera, int imgwidth, int imgheight, int imgdepth, const char *filename);
    void getStat(long int stat);   // Set flag to return a particular piece of data next frame
    void addDest(const char *host, int port);
    void rmDest(const char *host, int port);
    void rmAllDest();
};
#endif
