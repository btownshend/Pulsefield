import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

public class Max extends Synth {
	OscP5 oscP5;
	NetAddress MXaddr;

	Max(PApplet parent, OscP5 oscP5, NetAddress MXaddr) {
		super(parent);
		this.oscP5=oscP5;
		this.MXaddr=MXaddr;
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,MXaddr);
	}

	public void play(int id, int pitch, int velocity, int duration, int channel) {
		OscMessage msg=new OscMessage("/pf/pass/playmidinote");
		msg.add(id);
		msg.add(pitch);
		msg.add(velocity);
		msg.add(duration);
		msg.add(channel);
		sendMessage(msg);
	}

	public boolean handleMessage(OscMessage msg) {
		//PApplet.println("Max message: "+msg.toString());
		if (super.handleMessage(msg))
			return true;
		PApplet.println("Unknown Max Message: "+msg.toString());
		return false;
	}
}
