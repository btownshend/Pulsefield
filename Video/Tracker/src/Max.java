import java.util.HashMap;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

class MidiProgram {
	int instrument;
	String name;
	MidiProgram(int i, String s) { instrument=i; name=s; }
}

public class Max {
	static Max theMax;
	OscP5 oscP5;
	NetAddress MXaddr;
	HashMap<Integer,MidiProgram> channelmap;
	
	Max(OscP5 oscP5, NetAddress MXaddr) {
		this.oscP5=oscP5;
		this.MXaddr=MXaddr;
		theMax=this;
		channelmap=new HashMap<Integer,MidiProgram>();
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,MXaddr);
	}

	static public void play(int id, int pitch, int velocity, int duration, int channel) {
		OscMessage msg=new OscMessage("/pf/pass/playmidinote");
		msg.add(id);
		msg.add(pitch);
		msg.add(velocity);
		msg.add(duration);
		msg.add(channel);
		theMax.sendMessage(msg);
	}

	public static Max getInstance() {
		return theMax;
	}

	public void handleMessage(OscMessage msg) {
		//PApplet.println("Max message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (components.length==3 && components[1].equals("midi") && components[2].equals("pgm")) {
			int channel=msg.get(0).intValue();
			int instrument=msg.get(1).intValue();
			String pgmname=msg.get(2).stringValue();
			channelmap.put(channel,new MidiProgram(instrument,pgmname));
		} else
			PApplet.println("Unknown Max Message: "+msg.toString());
	}

	static public MidiProgram getMidiProgam(int ch) {
		return theMax.channelmap.get(ch);
	}
}
