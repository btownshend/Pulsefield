import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import javax.sound.midi.*;

import oscP5.OscMessage;
import processing.core.PApplet;

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

public class Synth {
	final String midiPortName="From Tracker";
	MidiDevice outputDevice=null;
	Receiver rcvr=null;
	//MidiBus myBus;
	Timer timer;
	HashMap<Integer,HashMap<Integer,NoteOff>> playing;    // Notes playing indexed by playing.get(channel).get(pitch)
	HashMap<Integer,MidiProgram> channelmap;

	Synth(PApplet parent)  {
		channelmap=new HashMap<Integer,MidiProgram>();
		outputDevice=getMidiDevice(midiPortName);
		if (outputDevice==null) {
			System.err.println("Unable to find MIDI device "+midiPortName);
			System.exit(1);
		}
		System.out.println("Connected to MIDI device: "+outputDevice.getDeviceInfo().getDescription());
		System.out.println(" getMicrosecondPosition-> "+outputDevice.getMicrosecondPosition());
		try {
			outputDevice.open();
			rcvr = outputDevice.getReceiver();
		} catch (MidiUnavailableException e1) {
			System.err.println("Unable to get receiver: "+e1);
			System.exit(1);
		}
		timer = new Timer();
		playing=new HashMap<Integer,HashMap<Integer,NoteOff>>();
		play(0,64,100,100,1);
	}

	public static MidiDevice getMidiDevice(String name) {
		MidiDevice.Info[] info = MidiSystem.getMidiDeviceInfo();
		for (MidiDevice.Info element : info) {
			System.out.println(element.getName()+" "+element.getDescription()+" "+element.getClass());
			if (element.getName().equals(name)) {
				try {
					MidiDevice dev=MidiSystem.getMidiDevice(element);
					if (dev.getMaxReceivers()!=0)
						return dev;
					System.out.println("No receivers");
				} catch (MidiUnavailableException e) {
					System.err.println("Midi device unavailable: "+name);
					System.exit(1);
				}
			}
		}
		return null;
	}


	public void play(int id, int pitch, int velocity, int duration, int channel)  {
		assert(channel>=0 && channel<16);
		long delay=duration*1000/480/4;
		if (playing.get(channel)==null)
			playing.put(channel, new HashMap<Integer,NoteOff>());
		if (playing.get(channel).get(pitch)!=null)
			playing.get(channel).get(pitch).cancel();
		// Check again in case there was a race
		if (playing.get(channel).get(pitch)!=null) {
			//System.out.println("Already playing note "+pitch+" on channel "+channel+", removing pending note-off");
			playing.get(channel).get(pitch).cancel();
		} else {
			ShortMessage myMsg = new ShortMessage();
			// Start playing the note Middle C (60), 
			// moderately loud (velocity = 93).
			try{
				myMsg.setMessage(ShortMessage.NOTE_ON, channel, pitch, velocity);
				long timeStamp = outputDevice.getMicrosecondPosition()+1000;
				System.out.println("timeStamp="+timeStamp);
				rcvr.send(myMsg, timeStamp);
			} catch (InvalidMidiDataException e) {
				System.err.println("Invalid MIDI data: "+e);
				System.exit(1);
			}
		}
		NoteOff noteOff=new NoteOff(this, pitch,64,channel);
		playing.get(channel).put(pitch, noteOff);
		timer.schedule(noteOff, delay);
		//System.out.println("Sent note "+pitch+", vel="+velocity+" , duration="+delay+"ms to channel "+channel);
	}

	public void endnote(int channel, int pitch, int velocity)  {
		if (playing.get(channel).get(pitch)==null)
			System.out.println("Received endnote for note that isn't playing; channel="+channel+", pitch="+pitch);
		try{
			ShortMessage myMsg = new ShortMessage();
			// Start playing the note Middle C (60), 
			// moderately loud (velocity = 93).
			myMsg.setMessage(ShortMessage.NOTE_OFF, channel,pitch,velocity);
			long timeStamp = outputDevice.getMicrosecondPosition();
			rcvr.send(myMsg, timeStamp);
		} catch (InvalidMidiDataException e) {
			System.err.println("Invalid MIDI data: "+e);
			System.exit(1);
		}

		playing.get(channel).remove(pitch);
		//System.out.println("Sent note off "+pitch+", vel="+velocity+" , to channel "+channel+", now have "+playing.get(channel).size()+" notes playing on this channel");
	}

	public MidiProgram getMidiProgam(int ch) {
		return channelmap.get(ch);
	}

	public void setCC(int channel, int cc, int value)  {
		try {
			ShortMessage myMsg = new ShortMessage();
			// Start playing the note Middle C (60), 
			// moderately loud (velocity = 93).
			myMsg.setMessage(ShortMessage.CONTROL_CHANGE, channel,cc,value);
			long timeStamp = -1;
			rcvr.send(myMsg, timeStamp);
		} catch (InvalidMidiDataException e) {
			System.err.println("Invalid MIDI data: "+e);
			System.exit(1);
		}
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
}
