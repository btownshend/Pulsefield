package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Logger;

import oscP5.OscMessage;

class MidiProgram {
	int instrument;
	String name;
	MidiProgram(int i, String s) { instrument=i; name=s; }
}

class NoteOff extends TimerTask {
	int pitch;
	int channel;
	int velocity;
	Synth synth;

	NoteOff(Synth synth, int pitch, int velocity, int channel) {
		this.synth=synth;
		this.pitch=pitch;
		this.velocity=velocity;
		this.channel=channel;
	}
	public void run() {
		synth.endnote(channel, pitch, velocity);
	}
}


abstract public class Synth {

	protected Timer timer;
	protected HashMap<Integer,HashMap<Integer,NoteOff>> playing;
	protected HashMap<Integer,MidiProgram> channelmap;
    private final static Logger logger = Logger.getLogger(Synth.class.getName());

	public Synth() {
		super();
		channelmap=new HashMap<Integer,MidiProgram>();
		timer = new Timer();
		playing=new HashMap<Integer,HashMap<Integer,NoteOff>>();
	}

	abstract public void play(int id, int pitch, int channel);

	// Play a note with given pitch(0-127), velocity(0-127), duration(msec), channel(0-15)
	public void play(int id, int pitch, int velocity, int duration,
			int channel) {
		if(channel<0 && channel>=16) {
			System.err.println("Warning: Bad channel: "+channel);
		}
		TrackSet ts=Ableton.getInstance().trackSet;
		if (ts==null) {
			System.err.println("synth.play: no trackSet");
			return;
		}
		int track=ts.getTrack(channel);
		playOnTrack(track,pitch,velocity,duration);
	}
	
	public void playOnTrack(int track, int pitch, int velocity, int duration) {
		long delay=duration*1000/480/4;
		if (playing.get(track)==null)
			playing.put(track, new HashMap<Integer,NoteOff>());
		if (playing.get(track).get(pitch)!=null)
			playing.get(track).get(pitch).cancel();
		// Check again in case there was a race
		if (playing.get(track).get(pitch)!=null) {
			//logger.warning("Already playing note "+pitch+" on channel "+channel+", removing pending note-off");
			playing.get(track).get(pitch).cancel();
		} else {
			play(pitch,velocity,track);
		}

		if (duration==0)
			endnote(track,pitch,64);
		else {
			NoteOff noteOff=new NoteOff(this, pitch,64,track);
			playing.get(track).put(pitch, noteOff);
			timer.schedule(noteOff, delay);
		}
		//logger.fine("Sent note "+pitch+", vel="+velocity+" , duration="+delay+"ms to track "+track+" for channel "+channel);
	}

	// Stop all currently playing notes
	public void endallnotes() {
		for (Map.Entry<Integer,HashMap<Integer,NoteOff>> t: playing.entrySet()) {
			for (Map.Entry<Integer, NoteOff> n: t.getValue().entrySet()) {
				logger.fine("Endallnotes: track "+t.getKey()+", pitch "+n.getKey());
				play(n.getKey(),0,t.getKey());
			}
		}
	}
	
	public void endnote(int track, int pitch, int velocity) {
		if (playing.get(track).get(pitch)==null)
			logger.warning("Received endnote for note that isn't playing; channel="+track+", pitch="+pitch);
		play(pitch,0,track);
		playing.get(track).remove(pitch);
		//logger.fine("Sent note off "+pitch+", vel="+velocity+" , to channel "+channel+", now have "+playing.get(channel).size()+" notes playing on this channel");
	}

	public boolean handleMessage(OscMessage msg) {
		//logger.fine("Synth message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (components.length==3 && components[1].equals("midi") && components[2].equals("pgm")) {
			int channel=msg.get(0).intValue();
			int instrument=msg.get(1).intValue();
			String pgmname=msg.get(2).stringValue();
			channelmap.put(channel,new MidiProgram(instrument,pgmname));
			return true;
		} 
		return false;
	}

	public MidiProgram getMidiProgam(int ch) {
		return channelmap.get(ch);
	}

	/** 
	 * Get the note namefor a midi pitch
	 * @param midiPitch midi pitch number
	 * @return note name  (e.g. "C3") 
	 */
	public static String pitchToName(int midiPitch) {
		final String keys[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
		int octave=(int)(midiPitch/12);
		int note=midiPitch-octave*12;
		return String.format("%s%d", keys[note],octave);
	}
	
	/** 
	 * Get the midi pitch for a note name 
	 * @param name not name (e.g. "C3")
	 * @return midi pitch number (0-127)
	 */
	public static int nameToPitch(String name) {
		final String keys[]={"C","C#","Db","D","D#","Eb","E","F","F#","Gb","G","G#","Ab","A","A#","Bb","B","B#","Cb"};
		final int keyv[]={60,61,61,62,63,63,64,65,66,66,67,68,68,69,70,70,71,72,72};
		int octave=5;
		char lastChar=name.charAt(name.length()-1);
		if (lastChar>='0' && lastChar<='9') {
			octave=(int)(lastChar-'0');
			name=name.substring(0, name.length()-1);
		}
		for (int i=0;i<keys.length;i++) {
			if (name.equals(keys[i]))
				return keyv[i]+(octave-5)*12;
		}
		logger.warning("Unable to parse note name: "+name);
		return -1;
	}
}
