import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;



public class TouchOSC {
	OscP5 oscP5;
	NetAddress addr;
	static TouchOSC theTouchOSC;
	
	TouchOSC(OscP5 oscP5, NetAddress ALaddr) {
		this.oscP5=oscP5;
		addr=ALaddr;
		theTouchOSC=this;
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,addr);
	}

	public static TouchOSC getInstance() {
		return theTouchOSC;
	}

}
