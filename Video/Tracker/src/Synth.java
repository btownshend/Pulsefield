import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

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

	public Synth() {
		super();
		channelmap=new HashMap<Integer,MidiProgram>();
		timer = new Timer();
		playing=new HashMap<Integer,HashMap<Integer,NoteOff>>();
	}

	abstract public void play(int id, int pitch, int channel);

	public void play(int id, int pitch, int velocity, int duration,
			int channel) {
		assert(channel>=0 && channel<16);
		TrackSet ts=Ableton.getInstance().trackSet;
		if (ts==null) {
			System.err.println("synth.play: no trackSet");
			return;
		}
		int track=ts.getTrack(channel);
		long delay=duration*1000/480/4;
		if (playing.get(track)==null)
			playing.put(track, new HashMap<Integer,NoteOff>());
		if (playing.get(track).get(pitch)!=null)
			playing.get(track).get(pitch).cancel();
		// Check again in case there was a race
		if (playing.get(track).get(pitch)!=null) {
			//System.out.println("Already playing note "+pitch+" on channel "+channel+", removing pending note-off");
			playing.get(track).get(pitch).cancel();
		} else {
			play(pitch,velocity,track);
		}
		NoteOff noteOff=new NoteOff(this, pitch,64,track);
		playing.get(track).put(pitch, noteOff);
		timer.schedule(noteOff, delay);
		//System.out.println("Sent note "+pitch+", vel="+velocity+" , duration="+delay+"ms to track "+track+" for channel "+channel);
	}

	public void endnote(int track, int pitch, int velocity) {
		if (playing.get(track).get(pitch)==null)
			System.out.println("Received endnote for note that isn't playing; channel="+track+", pitch="+pitch);
		play(pitch,0,track);
		playing.get(track).remove(pitch);
		//System.out.println("Sent note off "+pitch+", vel="+velocity+" , to channel "+channel+", now have "+playing.get(channel).size()+" notes playing on this channel");
	}

	public boolean handleMessage(OscMessage msg) {
		//PApplet.println("Synth message: "+msg.toString());
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

}