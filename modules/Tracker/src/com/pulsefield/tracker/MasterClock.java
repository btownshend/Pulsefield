package com.pulsefield.tracker;

public class MasterClock {
	static long lastBeatTime=-1;
	static float tempo=120;
	static float lastBeatValue=0;
	
	static float getBeat() {
		long now=System.currentTimeMillis();
		if (lastBeatTime==-1)
			lastBeatTime=now;
		lastBeatValue=(now-lastBeatTime)/1000f*(tempo/60)+lastBeatValue;
		lastBeatTime=now;
		return lastBeatValue;
	}
	
	static void settempo(float newTempo) {
		getBeat();  // Force update of current beat
		tempo=newTempo;
	}
	
	static float gettempo() {
		return tempo;
	}
}
