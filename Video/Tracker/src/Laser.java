import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;



public class Laser {
	OscP5 oscP5;
	NetAddress addr;
	static Laser theLaser;
	
	Laser(OscP5 oscP5, NetAddress addr) {
		PApplet.println("Laser destination set to "+addr);
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
	public void setFlag(String flag, float value) {
		OscMessage msg = new OscMessage("/ui/laser/"+flag);
		msg.add(value);
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
	
	public void svgfile(String path,float x, float y, float scaling, float rotateDeg) {
		OscMessage msg = new OscMessage("/laser/svgfile");
		msg.add(path);
		msg.add(x);
		msg.add(y);
		msg.add(scaling);
		msg.add(rotateDeg);
		sendMessage(msg);
	}
	
	public void circle(float x1,float y1, float r) {
		OscMessage msg = new OscMessage("/laser/circle");
		msg.add(x1);
		msg.add(y1);
		msg.add(r);
		sendMessage(msg);
	}
	
	public void cubic(float x1,float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
		OscMessage msg = new OscMessage("/laser/bezier/cubic");
		msg.add(x1);
		msg.add(y1);
		msg.add(x2);
		msg.add(y2);
		msg.add(x3);
		msg.add(y3);
		msg.add(x4);
		msg.add(y4);
		sendMessage(msg);
	}
	
	public void rect(float x, float y, float width, float height) {
		line(x,y,x+width,y);
		line(x+width,y,x+width,y+height);
		line(x+width,y+height,x,y+height);
		line(x,y+height,x,y);
	}
}
