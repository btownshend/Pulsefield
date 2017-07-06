package com.pulsefield.tracker;
import java.util.logging.Logger;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

public class Max extends Synth {
	OscP5 oscP5;
	NetAddress MXaddr;
    private final static Logger logger = Logger.getLogger(Max.class.getName());

	Max(PApplet parent, OscP5 oscP5, NetAddress MXaddr) {
		super();
		this.oscP5=oscP5;
		this.MXaddr=MXaddr;
		logger.config("MXaddr="+MXaddr);
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,MXaddr);
	}

	public void play(int pitch, int velocity, int track) {
		OscMessage msg=new OscMessage("/midi/note");
		msg.add(pitch);
		msg.add(velocity);
		msg.add(track);
		sendMessage(msg);
		//logger.fine("Send to MAX: play("+pitch+","+velocity+","+track+")");
	}


	public boolean handleMessage(OscMessage msg) {
		//logger.fine("Max message: "+msg.toString());
		if (super.handleMessage(msg))
			return true;
		logger.warning("Unknown Max Message: "+msg.toString());
		return false;
	}
}
