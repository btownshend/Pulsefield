import javax.sound.midi.*;

import processing.core.PApplet;


public class MidiSynth extends Synth {
	final String midiPortName="From Tracker";
	MidiDevice outputDevice=null;
	Receiver rcvr=null;
	MidiSynth(PApplet parent)  {
		super();
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


	public void play(int pitch, int velocity, int channel) {
		ShortMessage myMsg = new ShortMessage();
		// Start playing the note Middle C (60), 
		// moderately loud (velocity = 93).
		try{
			myMsg.setMessage(ShortMessage.NOTE_ON, channel, pitch, velocity);
			long timeStamp = -1;
			//System.out.println("timeStamp="+timeStamp);
			rcvr.send(myMsg, timeStamp);
		} catch (InvalidMidiDataException e) {
			System.err.println("Invalid MIDI data: "+e);
			System.exit(1);
		}
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
}
