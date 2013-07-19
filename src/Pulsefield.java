import oscP5.*;
import netP5.*;

import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PVector;

public class Pulsefield {
	OscP5 oscP5;
	NetAddress myRemoteLocation;
	float minx=-3.2f, maxx=3.2f, miny=-3.2f, maxy=3.2f;
	HashMap<Integer, Position> positions;
	PApplet parent;


	Pulsefield(PApplet parent) {
		this.parent=parent;
		/* start oscP5, listening for incoming messages from Pulsefield RealTime.m */
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
		positions = new HashMap<Integer,Position>();
	}


	/* incoming osc message are forwarded to the oscEvent method. */
	void oscEvent(OscMessage theOscMessage) {
		if (theOscMessage.isPlugged() == false) {
			PApplet.print("### Received an unhandled message: ");
			theOscMessage.print();
		}  /* print the address pattern and the typetag of the received OscMessage */
	}

	PVector mapposition(float x, float y) {
		return new PVector((int)((x-minx)/(maxx-minx)*parent.width), (int)((y-miny)/(maxy-miny)*parent.height) );
	}

	void draw() {
	}

	public void pfstarted() {
		PApplet.println("PF started");
	}

	public void pfstopped() {
		PApplet.println("PF stopped");
		positions.clear();
	}

	void frame(int frame) {
		PApplet.println("Got frame "+frame);
	}

	synchronized Position add(int id, int channel) {
		Position ps=new Position();
		positions.put(id,ps);
		return ps;
	}

	synchronized public void pfupdate(int sampnum, float elapsed, int id, float ypos, float xpos, float yvelocity, float xvelocity, float majoraxis, float minoraxis, int groupid, int groupsize, int channel) {
		if (channel!=99) {
			PApplet.print("update: ");
			PApplet.print("samp="+sampnum);
			PApplet.print(",elapsed="+elapsed);
			PApplet.print(",id="+id);
			PApplet.print(",pos=("+xpos+","+ypos+")");
			PApplet.print(",vel=("+xvelocity+","+yvelocity+")");
			PApplet.print(",axislength=("+majoraxis+","+minoraxis+")");
			PApplet.println(",channel="+channel);
		}

		Position ps=positions.get(id);
		if (ps==null) {
			PApplet.println("Unable to locate user "+id+", creating it.");
			ps=add(id,channel);
		}

		ps.move(mapposition(xpos, ypos), elapsed);
		ps.enable(true);

	}

	// Version for simplified access (primarily for testing)
	public void pfupdate(float elapsed, int id, float xpos, float ypos) {
		pfupdate(0, elapsed, id, ypos*1.0f/parent.height*(maxy-miny)+miny, xpos*1.0f/parent.width*(maxx-minx)+minx, 0, 0, 0, 0, 1, 1, id);
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
		if (positions.containsKey(id)) {
			positions.get(id).enable(false);
		} 
		else
			PApplet.println("Unable to locate particle system "+id);
	}


	synchronized public void pfentry(int sampnum, float elapsed, int id, int channel) {
		PApplet.println("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel);
		add(id,channel);
	}
}

