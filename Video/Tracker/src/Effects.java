import java.util.ArrayList;
import java.util.HashMap;

import processing.core.PApplet;

// Effects using a midi map
class Effects {
	Synth synth;
	int track;
	HashMap<String,ArrayList<Integer>> pitchMap;
	static public Effects defaultEffects=null;
	static int DEFAULTEFFECTSTRACK=127;
	
	Effects(Synth synth, int track) {
		this.synth=synth;
		this.track=track;		
		pitchMap=new HashMap<String,ArrayList<Integer>>();
	}

	void add(String effect, String pitchName) {
		int pitch=Synth.nameToPitch(pitchName)+24;  // Seems like Ableton naming is C3=60 instead of C5=60
		add(effect,pitch);
	}
	void add(String effect, int pitch) {
		if (!pitchMap.containsKey(effect))
			pitchMap.put(effect,new ArrayList<Integer>());
		pitchMap.get(effect).add(pitch);
	}
	void add(String effect, int pitch1, int pitch2) {
		for (int i=pitch1;i<=pitch2;i++)
			add(effect,i);
	}
	public void play(String effect, int velocity, int duration) {
		if (!pitchMap.containsKey(effect)) {
			PApplet.println("No pitchMap entry for effect: "+effect);
			return;
		}
		ArrayList<Integer> pitches=pitchMap.get(effect);
		int id=(int)(Math.random()*pitches.size());
		int pitch=pitches.get(id%pitches.size());
		PApplet.println("Effect ("+effect+"):  id: "+id+", pitch: "+pitch+", velocity: "+velocity+", track: "+track);
		synth.playOnTrack(track, pitch, velocity, duration);
	}
	
	static void setupDefault(Synth synth) {
		// Setup default effects
		defaultEffects = new Effects(synth,DEFAULTEFFECTSTRACK);
		// Whack 
		defaultEffects.add("WHACK","C1");
		defaultEffects.add("Smash", "C1");
		// DNA
		defaultEffects.add("BREAK","C1");
		defaultEffects.add("LIGATE","C1");
	}
}