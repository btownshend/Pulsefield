// Track music beats, etc
#pragma once

#include <sys/time.h>
#include "touchosc.h"

class Music {
    static Music *theInstance;
    float bpm;
    static const int beatsperbar=4;
    float fracbar;    // Position in bars
    struct timeval(lastbeattime);
    lo_address remote;
    Music();
 public:
    static Music *instance() {
	if (theInstance == NULL) {
	    theInstance=new Music();
	}
	return theInstance;
    }
    int handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg);
    void predict() { 
	struct timeval now;
	gettimeofday(&now,0);
	float elapsed=(now.tv_sec-lastbeattime.tv_sec)+(now.tv_usec-lastbeattime.tv_usec)/1e6;
	fracbar+=bpm/60*elapsed/beatsperbar;
	lastbeattime=now;
	dbg("Music.predict",3) << "Current pos=" << fracbar << " elapsed=" << elapsed <<  "bpm=" << bpm <<  std::endl;
    }
    int getLastBar() const { return (int)fracbar; }
    int getBar() const {
	((Music *)this)->predict(); return getLastBar(); 
    }
    float getLastBeat() const {
	return fmod(fracbar,1.0)*beatsperbar + 1;
    }
    float getFracBar() const {
	((Music *)this)->predict(); 
	return fmod(fracbar,1.0);
    }
    float getBeat() const {
	((Music *)this)->predict(); 
	return getLastBeat();
    }
    int getTempo() const { return bpm; }
    void setBeat(int _bar, int _beat) {
	float oldfracbar=fracbar;
	gettimeofday(&lastbeattime,0); 
	fracbar=_bar+(_beat-1)*1.0/beatsperbar;
	dbg("Music.setBeat",1) << "beat=" << _beat << ", bar=" << _bar << ", fracbar: " << oldfracbar << " -> " << fracbar << std::endl;
    }
    void setTempo(float _bpm)  { bpm=_bpm; }

    int send(std::string path, float value) const;
    int send(std::string path, std::string value) const;

    void frameTick(int frame);
};
