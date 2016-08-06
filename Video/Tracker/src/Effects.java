import java.util.HashMap;

import processing.core.PApplet;

class Effects {
	Synth synth;
	HashMap<String,Integer[]> pitchMap;
	
	Effects(Synth synth) {
		this.synth=synth;
		pitchMap=new HashMap<String,Integer[]>();
	}
	void put(String effect, Integer pitch[]) {
		pitchMap.put(effect,pitch);
	}
	
	public void play(Person p, String effect, int velocity, int duration) {
		if (!pitchMap.containsKey(effect)) {
			PApplet.println("No pitchMap entry for effect: "+effect);
			return;
		}
		Integer pitches[]=pitchMap.get(effect);
		int pitch=pitches[p.id%pitches.length];
		PApplet.println("Effect ("+effect+"):  id: "+p.id+", pitch: "+pitch+", velocity: "+velocity);
		synth.play(p.id, pitch, velocity, duration, p.channel);
	}
}