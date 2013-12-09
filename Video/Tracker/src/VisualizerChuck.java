import java.util.HashMap;
import java.util.Iterator;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

// Visualizer that sends messages to chuck
class Fiducial extends Position {
	int type;
	Fiducial parent;
	
	Fiducial(Position ps, int type) {
		super(ps.origin,ps.channel,ps.id);
		this.type=type;
		OscMessage msg = new OscMessage("/chuck/new");
		msg.add(id);
		msg.add(type);
		Tracker.sendOSC("CK",msg);
	}
	
	void update(Position ps) {
		origin=ps.origin;
		OscMessage msg = new OscMessage("/chuck/update");
		msg.add(type);
		msg.add(ps.origin.x);
		msg.add(ps.origin.y);
		Tracker.sendOSC("CK",msg);
	}
	

	void draw(PApplet parent, PVector wsize, float sz) {
		float x=(origin.x+1)*wsize.x/2;
		float y=(origin.y+1)*wsize.y/2;
		if (type==0) {
			parent.ellipseMode(PConstants.CENTER);
			parent.ellipse(x, y, sz, sz);
		} else if (type==1) {
			parent.rectMode(PConstants.CENTER);
			parent.rect(x, y, sz, sz);
		} else if (type==2) {
			parent.triangle(x-sz/2,y+sz/2,x+sz/2,y+sz/2,x,y-sz/2);
		} else {
			PApplet.println("Unable to draw fiducial type "+type);
		}
	}
}

class Fiducials extends HashMap<Integer,Fiducial> {
	private static final long serialVersionUID = -1131006311643745996L;
	static final String types[]= {"Carrier","Modulator","MI"};
	static final int ntypes=types.length;


	Fiducials() {
		super();
	}
	
	// Choose a type for a new entry
	int newType(int id) {
		int newt=id%ntypes;
		return newt;
	}
	
	/* Make links from fiducials to lower numbered ones */
	void makeLinks() {
		for (Fiducial f: values()) {
			if (f.type>0) {
				float mindist=1e10f;
				Fiducial newparent=null;
				for (Fiducial f2: values()) {
					if (f2.type==f.type-1) {
						float dist=PVector.dist(f.origin, f2.origin);
						//PApplet.println("Distance from "+f.id+" to "+f2.id+" = "+dist);
						if (dist<mindist) {
							mindist=dist;
							newparent=f2;
						}
					}
				}

				if (f.parent!=newparent && f.parent!=null) {
					// Remove old link
					PApplet.println("Disconnect "+f.parent.id+" from "+f.id);
					OscMessage msg = new OscMessage("/chuck/disconnect");
					msg.add(f.id);
					Tracker.sendOSC("CK",msg);
					f.parent=null;
				}
				if (newparent!=null && f.parent==null) {
					f.parent=newparent;
					OscMessage msg = new OscMessage("/chuck/connect");
					msg.add(f.id);
					msg.add(f.parent.id);	
					Tracker.sendOSC("CK",msg);
					PApplet.println("Connect "+f.parent.id+" to "+f.id);
				}
			}
		}
	}
	
	void draw(PApplet parent, PVector wsize) {
		float sz=20;
		for (Fiducial f: values()) {
			int c=f.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			parent.strokeWeight(5);
			f.draw(parent,wsize,sz);
			if (f.parent!=null) {
				parent.line((f.origin.x+1)*wsize.x/2, (f.origin.y+1)*wsize.y/2, (f.parent.origin.x+1)*wsize.x/2, (f.parent.origin.y+1)*wsize.y/2);
			}
		}
	}
}

public class VisualizerChuck extends Visualizer {
	Fiducials fiducials;
	
	VisualizerChuck(PApplet parent) {
		super();
		fiducials=new Fiducials();
	}
	


	@Override
	public void update(PApplet parent, Positions allpos) {
		// Update internal state of the dancers
		for (int id: allpos.positions.keySet()) {
			if (!fiducials.containsKey(id)) {
				fiducials.put(id,new Fiducial(allpos.get(id),fiducials.newType(id)));
			}
			
			Fiducial f=fiducials.get(id);
			f.update(allpos.get(id));
		}
		// Remove fiducials for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = fiducials.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.positions.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				iter.remove();
			}
		}
		fiducials.makeLinks();
	}

	@Override
	public void start() {
		OscMessage msg = new OscMessage("/chuck/start");
		Tracker.sendOSC("CK",msg);
	}
	
	@Override
	public void draw(PApplet parent, Positions p, PVector wsize) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		super.draw(parent, p, wsize);

		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0);
		drawBorders(parent,true,wsize);

		fiducials.draw(parent,wsize);
	}
}

