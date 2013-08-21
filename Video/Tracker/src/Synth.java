import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import oscP5.OscMessage;
import processing.core.PApplet;
import themidibus.MidiBus;

class MidiProgram {
	int instrument;
	String name;
	MidiProgram(int i, String s) { instrument=i; name=s; }
}

class NoteOff extends TimerTask {
	int pitch;
	int channel;
	int velocity;
	MidiBus bus;
	
	NoteOff(MidiBus bus, int pitch, int velocity, int channel) {
		this.bus=bus;
		this.pitch=pitch;
		this.velocity=velocity;
		this.channel=channel;
	}
	public void run() {
		bus.sendNoteOff(channel, pitch, velocity);
		//System.out.println("Sent note off "+pitch+", vel="+velocity+" , to channel "+channel);
	}
}

public class Synth {
	final String midiPortName="From Tracker";
	MidiBus myBus;
	Timer timer;
	
	HashMap<Integer,MidiProgram> channelmap;

	Synth(PApplet parent) {
		channelmap=new HashMap<Integer,MidiProgram>();

		//print a list with all available devices
		for (String s:MidiBus.availableOutputs())
			PApplet.println(s);

		myBus = new MidiBus(parent, -1, "From Tracker");
		timer = new Timer();
		play(0,64,100,100,1);
	}

	public void play(int id, int pitch, int velocity, int duration, int channel) {
		assert(channel>=0 && channel<16);
		myBus.sendNoteOn(channel,pitch,velocity);
		long delay=duration*1000/480/4;
		timer.schedule(new NoteOff(myBus, pitch,0,channel), delay);
		System.out.println("Sent note "+pitch+", vel="+velocity+" , duration="+delay+"ms to channel "+channel);
	}

	public MidiProgram getMidiProgam(int ch) {
		return channelmap.get(ch);
	}

	public void setCC(int channel, int cc, int value) {
		myBus.sendControllerChange(channel, cc, value);
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
