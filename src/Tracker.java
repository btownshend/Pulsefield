import java.awt.Color;

import processing.core.PApplet;
import oscP5.*;
import netP5.*;
import processing.core.PVector;

public class Tracker extends PApplet {
	/**
	 * 
	 */
	private static boolean present = false;

	private static final long serialVersionUID = 1L;
	int tick=0;
	private float avgFrameRate=0;
	OscP5 oscP5;
	NetAddress myRemoteLocation;
	float minx=-3.2f, maxx=3.2f, miny=-3.2f, maxy=3.2f;
	Visualizer vis[];
	String visnames[]={"Smoke","Navier","Tron"};
	String vispos[]={"5/1","5/2","5/3"};
	int currentvis;
	NetAddress touchOSC;

	public void setup() {
		size(640,400, OPENGL);
		frameRate(60);
		frame.setBackground(new Color(0,0,0));
		oscP5 = new OscP5(this, 7002);
		oscP5.plug(this, "pfframe", "/pf/frame");
		oscP5.plug(this, "pfupdate", "/pf/update");
		oscP5.plug(this, "pfsetnpeople", "/pf/set/npeople");
		oscP5.plug(this, "pfexit", "/pf/exit");
		oscP5.plug(this, "pfentry", "/pf/entry");
		oscP5.plug(this, "pfsetminx", "/pf/set/minx");
		oscP5.plug(this, "pfsetminy", "/pf/set/miny");
		oscP5.plug(this, "pfsetmaxx", "/pf/set/maxx");
		oscP5.plug(this, "pfsetmaxy", "/pf/set/maxy");
		oscP5.plug(this, "pfstarted", "/pf/started");
		oscP5.plug(this, "pfstopped", "/pf/stopped");
		oscP5.plug(this, "vsetapp51", "/video/app/buttons/5/1");
		oscP5.plug(this, "vsetapp52", "/video/app/buttons/5/2");
		oscP5.plug(this, "vsetapp53", "/video/app/buttons/5/3");
		touchOSC = new NetAddress("192.168.0.148",9998);

		vis=new Visualizer[3];
		vis[0]=new VisualizerPS(this);
		vis[1]=new VisualizerNavier(this);
		vis[2]=new VisualizerTron(this);
		currentvis=2;
		setapp(currentvis);
	}

	public void vsetapp(OscMessage msg) {
		println("vsetup("+msg+")");
	}
	public void vsetapp51(float onoff) {
		setapp(0);
	}
	public void vsetapp52(float onoff) {
		setapp(1);
	}
	public void vsetapp53(float onoff) {
		setapp(2);
	}
		
	public void setapp(int appNum) {
		if (appNum <0 || appNum > vis.length) {
			println("Bad video app number: "+appNum);
			return;
		}
		// Turn off old block
		OscMessage msg = new OscMessage("/video/app/buttons/"+vispos[currentvis]);
		msg.add(0);
		oscP5.send(msg,touchOSC);


		currentvis=appNum;
		println("Switching to app "+currentvis+": "+visnames[currentvis]);
		// Turn on block for current app
		msg = new OscMessage("/video/app/buttons/"+vispos[currentvis]);
		msg.add(1.0);
		oscP5.send(msg,touchOSC);
		
		msg = new OscMessage("/video/app/name");
		msg.add(visnames[currentvis]);
		oscP5.send(msg,touchOSC);
	}

	synchronized public void draw() {
		tick++;
		avgFrameRate=avgFrameRate*(1f-1f/200f)+frameRate/200f;
		if (tick%200 == 0) {
			println("Average frame rate = "+avgFrameRate);
			vis[currentvis].stats();
		}

		if (mousePressed)
			vis[currentvis].move(98, 98, new PVector(mouseX*1f, mouseY*1f), tick/avgFrameRate);


		vis[currentvis].update(this);
		vis[currentvis].draw(this);
	}

	public void mouseReleased() {
		pfexit(0, 0, 98);
	}

	public static void main(String args[]) {
		if (present)
			PApplet.main(new String[] { "--present","Tracker" });
		else
			PApplet.main(new String[] { "Tracker" });

	}

	/* incoming osc message are forwarded to the oscEvent method. */
	public void oscEvent(OscMessage theOscMessage) {
		if (theOscMessage.checkAddrPattern("/video/app/buttons") == true)
			vsetapp(theOscMessage);
		
		if (theOscMessage.isPlugged() == false) {
			PApplet.print("### Received an unhandled message: ");
			theOscMessage.print();
		}  /* print the address pattern and the typetag of the received OscMessage */
	}

	PVector mapposition(float x, float y) {
		return new PVector((int)((x-minx)/(maxx-minx)*width), (int)((y-miny)/(maxy-miny)*height) );
	}

	public void pfstarted() {
		PApplet.println("PF started");
	}

	public void pfstopped() {
		PApplet.println("PF stopped");
		vis[currentvis].clear();

	}

	void pfframe(int frame) {
		PApplet.println("Got frame "+frame);
	}

	synchronized void add(int id, int channel) {
		vis[currentvis].add(id, channel);
	}

	synchronized public void pfupdate(int sampnum, float elapsed, int id, float ypos, float xpos, float yvelocity, float xvelocity, float majoraxis, float minoraxis, int groupid, int groupsize, int channel) {
	/*	if (channel!=99) {
			PApplet.print("update: ");
			PApplet.print("samp="+sampnum);
			PApplet.print(",elapsed="+elapsed);
			PApplet.print(",id="+id);
			PApplet.print(",pos=("+xpos+","+ypos+")");
			PApplet.print(",vel=("+xvelocity+","+yvelocity+")");
			PApplet.print(",axislength=("+majoraxis+","+minoraxis+")");
			PApplet.println(",channel="+channel);
		} */
		vis[currentvis].move(id, channel, mapposition(xpos, ypos), elapsed);
	}

	public void pfsetminx(float minx) {  
		this.minx=minx;
	}
	public void pfsetminy(float miny) {  
		this.miny=miny;
	}
	public void pfsetmaxx(float maxx) {  
		this.maxx=maxx;
	}
	public void pfsetmaxy(float maxy) {  
		this.maxy=maxy;
	}

	public void pfsetnpeople(int n) {
		PApplet.println("/pf/set/npeople: now have "+n+" people");
	}

	synchronized public void pfexit(int sampnum, float elapsed, int id) {
		PApplet.println("exit: sampnum="+sampnum+", elapsed="+elapsed+", id="+id);
		vis[currentvis].exit(id);
	}

	synchronized public void pfentry(int sampnum, float elapsed, int id, int channel) {
		PApplet.println("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel);
		add(id,channel);
	}

}

