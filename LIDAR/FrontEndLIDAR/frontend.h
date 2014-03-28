#ifndef _FRONTEND_H_
#define _FRONTEND_H_

#include <pthread.h>
#include "lo/lo.h"
#include "defs.h"
#include "dest.h"

class SickIO;
class World;
class Snapshot;
class Vis;

class FrontEnd {
    int serverPort;

    int nsick;
    int nechoes;
    Snapshot *snap;
    SickIO **sick;
    World *world;
    Vis *vis;
    Destinations dests;
    lo_server s;
    
    int frame;
    long int sendOnce, sendAlways;

    // Process all frames
    void processFrames();   
    // Send out low-level vis messages for given sick data
    void sendVisMessages(int id, unsigned int frame, const struct timeval &acquired, int nmeasure, int necho, const unsigned int *ranges[], const unsigned int *reflect[]);
    void recordFrame();

    bool recording;
    FILE *recordFD;

    pthread_t incomingThread;
    static void *processIncoming(void *arg);
 public:
    enum { RANGE=0x10, REFLECT=0x20 };  // Bitmasks of what to send in next message group

    FrontEnd(int nsick);
    ~FrontEnd();

    void run();

    int startRecording(const char *filename);
    void stopRecording();
    int playFile(const char *filename, bool singleStep,float speedFactor=1.0f);

    // Handlers for OSC messages
    void startStop(bool start);
    void setFPS(int fps);
    void setRes(int camera,const char *res);
    void getStat(long int stat,int mode);   // Set flag to return a particular piece of data next frame
    void setEchoes(int echoes);
    void addDest(const char *host, int port);
    void addDest(lo_message msg, int port);
    void rmDest(const char *host, int port);
    void rmDest(lo_message msg, int port);
    void rmAllDest();
    void ping(lo_message msg, int seqnum);
};
#endif
