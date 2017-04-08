#pragma once

#include <vector>
#include <string>
#include <pthread.h>
#include "lo/lo.h"
#include "dest.h"
#include "configuration.h"

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
    lo_address touchOSC;
    Configuration config;
    
    int frame;
    struct timeval currenttime; 	// Time of last acquired frame (set either during realtime operation or by using stored acquired time when reading from a file)
    long int sendOnce, sendAlways;

    // Start time of run (used to set zero reference)
    struct timeval starttime;

    // Process all frames
    void processFrames();   
    // Send out low-level vis messages for given sick data
    void sendVisMessages(int id, unsigned int frame, const struct timeval &acquired, int nmeasure, int necho, const unsigned int *ranges[], const unsigned int *reflect[]);
    void recordFrame();

    bool recording;
    FILE *recordFD;

    pthread_t incomingThread;
    static void *processIncoming(void *arg);
    int matframes;
    std::string matfile;
    std::vector<std::string> arglist;

    // OSC Message handling
    void addHandlers();
    void addSickHandlers(int i);
    static bool doQuit;

    // Startup messages to OSC
    void sendInitialMessages(const char *host, int port) const;
    void sendSetupMessages(const char *host, int port) const;
    // Current messages
    void sendMessages(double elapsed);
    void sendUIMessages();
 public:
    enum { RANGE=0x10, REFLECT=0x20, PF=0x40 };  // Bitmasks of what to send in next message group

    FrontEnd(int nsick,float maxRange,int argc, const char **argv);
    ~FrontEnd();

    void run();

    int startRecording(const char *filename);
    void stopRecording();
    int playFile(const char *filename, bool singleStep,float speedFactor=1.0f,bool overlayLive=false,int frame1=-1,int frameN=-1,bool savePerfData=false);

    void matsave(const std::string &filename, int frames);

    // Save/load configuration
    void save();
    void load();

    // Handlers for OSC messages
    void quit();
    void startStop(bool start);
    void setFPS(int fps);
    int getFPS() const;
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
