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
    Music() { bpm=120; setBeat(1,1); }
 public:
    static Music *instance() {
	if (theInstance == NULL) {
	    theInstance=new Music();
	}
	return theInstance;
    }
    void predict() { 
	struct timeval now;
	gettimeofday(&now,0);
	float elapsed=(now.tv_sec-lastbeattime.tv_sec)+(now.tv_usec-lastbeattime.tv_usec)/1e6;
	fracbar+=bpm/60*elapsed/beatsperbar;
	lastbeattime=now;
	dbg("Music.predict",3) << "Current pos=" << fracbar << " elapsed=" << elapsed <<  std::endl;
    }
    int getLastBar() const { return (int)fracbar; }
    int getBar() const {
	((Music *)this)->predict(); return getLastBar(); 
    }
    float getLastBeat() const {
	return fmod(fracbar,1.0)*beatsperbar + 1;
    }
    float getBeat() const {
	((Music *)this)->predict(); 
	return getLastBeat();
    }
    int getTempo() const { return bpm; }
    void setBeat(int _beat, int _bar) {
	fracbar=_bar+(_beat-1)/beatsperbar; gettimeofday(&lastbeattime,0); 
    }
    void setTempo(float _bpm)  { bpm=_bpm; }

};
