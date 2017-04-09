package com.pulsefield.tracker;
import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;



public class OFOSC {
	OscP5 oscP5;
	NetAddress addr;
	static OFOSC theOFOSC;
	
	OFOSC(OscP5 oscP5, NetAddress ALaddr) {
		this.oscP5=oscP5;
		addr=ALaddr;
		theOFOSC=this;
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,addr);
	}

	public static OFOSC getInstance() {
		return theOFOSC;
	}

}
