import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;



public class Laser {
	OscP5 oscP5;
	NetAddress addr;
	static Laser theLaser;
	
	Laser(OscP5 oscP5, NetAddress addr) {
		this.oscP5=oscP5;
		this.addr=addr;
		theLaser=this;
	}

	public void sendMessage(OscMessage msg) {
	//	PApplet.println("Laser send: "+msg.toString());
		oscP5.send(msg,addr);
	}

	public static Laser getInstance() {
		return theLaser;
	}

	public void cellBegin(int id) {
		OscMessage msg = new OscMessage("/laser/cell/begin");
		msg.add(id);
		sendMessage(msg);
	}
	public void cellEnd(int id) {
		OscMessage msg = new OscMessage("/laser/cell/end");
		msg.add(id);
		sendMessage(msg);
	}
	public void bgBegin() {
		OscMessage msg = new OscMessage("/laser/bg/begin");
		sendMessage(msg);
	}
	public void bgEnd() {
		OscMessage msg = new OscMessage("/laser/bg/end");
		sendMessage(msg);
	}

	public void line(float x1,float y1, float x2, float y2) {
		OscMessage msg = new OscMessage("/laser/line");
		msg.add(x1);
		msg.add(y1);
		msg.add(x2);
		msg.add(y2);
		sendMessage(msg);
	}
	public void rect(float x, float y, float width, float height) {
		line(x,y,x+width,y);
		line(x+width,y,x+width,y+height);
		line(x+width,y+height,x,y+height);
		line(x,y+height,x,y);
	}
}
