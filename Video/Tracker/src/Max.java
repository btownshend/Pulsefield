import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

public class Max extends Synth {
	OscP5 oscP5;
	NetAddress MXaddr;

	Max(PApplet parent, OscP5 oscP5, NetAddress MXaddr) {
		super();
		this.oscP5=oscP5;
		this.MXaddr=MXaddr;
		System.out.println("MXaddr="+MXaddr);
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
		System.out.println("Send to MAX: play("+pitch+","+velocity+","+track+")");
	}


	public boolean handleMessage(OscMessage msg) {
		//PApplet.println("Max message: "+msg.toString());
		if (super.handleMessage(msg))
			return true;
		PApplet.println("Unknown Max Message: "+msg.toString());
		return false;
	}
}
