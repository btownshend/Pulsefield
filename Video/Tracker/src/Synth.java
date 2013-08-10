import java.util.HashMap;

import oscP5.OscMessage;

import processing.core.PApplet;
import promidi.*;

class MidiProgram {
	int instrument;
	String name;
	MidiProgram(int i, String s) { instrument=i; name=s; }
}

public class Synth {
	MidiIO midiIO;
	MidiOut midiOut[];
	HashMap<Integer,MidiProgram> channelmap;

	@SuppressWarnings("unused")
	Synth(PApplet parent) {
		channelmap=new HashMap<Integer,MidiProgram>();
		//get an instance of MidiIO
		midiIO = MidiIO.getInstance(parent);

		//print a list with all available devices
		midiIO.printDevices();

		//open an midiout using the first device and the first channel
		midiOut=new MidiOut[16];
		for (int ch=0;ch<16;ch++)
			midiOut[ch] = midiIO.getMidiOut(ch, "From Tracker");

		// Figure out input device number
		int midiInDevNum=-1;
		final String inputDeviceName="From Noatikl";
		for (int i = 0; i < midiIO.numberOfInputDevices(); i++){
			if (midiIO.getInputDeviceName(i).equals(inputDeviceName)){
				midiInDevNum=i;
				break;
			}
		}
		if (midiInDevNum == -1)
			throw new RuntimeException("There is no input device with the name " + inputDeviceName + ".");
		System.out.println("Input device = "+midiInDevNum);
		if (false) {
			midiIO.plug(this, "gotNote", midiInDevNum, 0);
			midiIO.plug(this, "gotController", midiInDevNum, 0);
			midiIO.plug(this, "gotProgramChange", midiInDevNum, 0);
		}
		for (int i=0;i<16;i++)
			midiIO.openInput(midiInDevNum, i);
	}

	public void play(int id, int pitch, int velocity, int duration, int channel) {
		Note note = new Note(pitch,velocity,(int)(duration*1000*60/MasterClock.gettempo()/480));
		assert(channel>=0 && channel<16);
		midiOut[channel].sendNote(note);
		System.out.println("Sent note "+pitch+", vel="+velocity+" , to channel "+channel);
	}

	public MidiProgram getMidiProgam(int ch) {
		return channelmap.get(ch);
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
