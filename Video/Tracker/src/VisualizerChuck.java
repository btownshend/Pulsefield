import java.util.HashMap;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

// Visualizer that sends messages to chuck
abstract class Fiducial extends Position {
	Fiducial(Position ps) {
		super(ps.origin,ps.channel,ps.id);
	}

	void update(Position ps) {
		origin=ps.origin;
	}

	abstract void draw(PApplet parent, PVector wsize, float sz);
	void stop() { }
}

class CCPair {
	int cc1,cc2;
	int color;
	CCPair() {
		cc1=-1;
		cc2=-1;
	}
	CCPair(int cc1, int cc2, int color) {
		this.cc1=cc1;
		this.cc2=cc2;
		this.color=color;
	}
}

class GeneratorType {
	String name;
	int code;
	int color;
	CCPair cclist[];
	GeneratorType(String name, int color, int code, CCPair cc1) {
		this.name=name;
		this.color=color;
		this.code=code;
		this.cclist=new CCPair[2];
		cclist[0]=cc1;
		cclist[1]=new CCPair(-1,-1,0);
	}
	GeneratorType(String name, int color, int code, CCPair cc1, CCPair cc2) {
		this.name=name;
		this.color=color;
		this.code=code;
		this.cclist=new CCPair[2];
		cclist[0]=cc1;
		cclist[1]=cc2;
	}
}


class Generator extends Fiducial {
	static final GeneratorType genTypes[] = {
		new GeneratorType("FM",0xff0000ff,0,new CCPair(1,11,0xffff00ff)),
		new GeneratorType("BeeThree",0xffff0000,1,new CCPair(2,4,0xffffff00),new CCPair(1,11,0xffff00ff)),
		new GeneratorType("Mandolin",0xff00ff00,2,new CCPair(2,4,0xffffff00),new CCPair(1,11,0xffff00ff))
	};

	static final int NUMGENTYPES=genTypes.length;

	GeneratorType genType;
	int nControllers;

	Generator(Position ps, GeneratorType genType) {
		super(ps);
		this.genType=genType;
		OscMessage msg = new OscMessage("/chuck/new");
		msg.add(id);
		msg.add(genType.code);
		Tracker.sendOSC("CK",msg);
		nControllers=0;
	}
	
	@Override() 
	void stop() {
		OscMessage msg = new OscMessage("/chuck/del");
		msg.add(id);
		Tracker.sendOSC("CK",msg);
		PApplet.println("Stopped generator ID "+id);
	}

	@Override
	void update(Position ps) {
		super.update(ps);
		OscMessage msg = new OscMessage("/chuck/dev/"+id+"/pan");
		msg.add((ps.origin.x+1f)/2f);
		Tracker.sendOSC("CK",msg);
		msg = new OscMessage("/chuck/dev/"+id+"/y");
		msg.add((-ps.origin.y+1f)/2f); // Flip direction so top of screen (far side of PF) is 1.0
		Tracker.sendOSC("CK",msg);
	}

	@Override 
	void draw(PApplet parent, PVector wsize, float sz) {
		float x=(origin.x+1)*wsize.x/2;
		float y=(origin.y+1)*wsize.y/2;
		parent.fill(genType.color);
		parent.stroke(genType.color);
		parent.ellipseMode(PConstants.CENTER);
		parent.ellipse(x, y, sz, sz);
		parent.fill(255,255);
		parent.textSize(sz*.7f);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.text(genType.name,x,y);
	}
}

class Controller extends Fiducial {
	Generator parent;
	CCPair cc;   // Controller numbers to use for (dir,dist), -1 to ignore
	float cc1val, cc2val;
	
	Controller(Position ps) {
		super(ps);
		parent=null;
		cc=null;
	}

	@Override
	void update(Position ps) {
		super.update(ps);
		if (parent!=null) {
			float dir=PVector.sub(parent.origin,origin).heading();
			float dist=PVector.dist(parent.origin,origin);
			if (cc.cc1!=-1) {
				OscMessage msg = new OscMessage("/chuck/dev/"+parent.id+"/cc");
				msg.add(cc.cc1);
				cc1val = (float) (dir/(2*Math.PI)-0.25f);  // Map so up on screen is 0.0 and increases CW
				if (cc1val<0)cc1val=cc1val+1f;
				cc1val *= 128;
				msg.add(cc1val);
				Tracker.sendOSC("CK",msg);
			}
			if (cc.cc2!=-1) {
				OscMessage msg = new OscMessage("/chuck/dev/"+parent.id+"/cc");
				msg.add(cc.cc2);
				// Linear from 0.0 at DISTBREAK to 1.0 at distance DISTCREATE
				cc2val=128*(1.0f-dist/(Fiducials.DISTBREAK-Fiducials.DISTCREATE));
				if (cc2val>128.0f) cc2val=128.0f;
				if (cc2val<0.0f) cc2val=0.0f;
				msg.add(cc2val);  
				Tracker.sendOSC("CK",msg);
			}
		}
	}

	@Override 
	void draw(PApplet parent, PVector wsize, float sz) {
		float x=(origin.x+1)*wsize.x/2;
		float y=(origin.y+1)*wsize.y/2;
		if (cc!=null) {
			parent.fill(cc.color);
			parent.stroke(cc.color);
		}
		parent.rectMode(PConstants.CENTER);
		parent.rect(x, y, sz, sz);
		// parent.triangle(x-sz/2,y+sz/2,x+sz/2,y+sz/2,x,y-sz/2);
		if (this.parent!=null) {
			parent.line((origin.x+1)*wsize.x/2, (origin.y+1)*wsize.y/2, (this.parent.origin.x+1)*wsize.x/2, (this.parent.origin.y+1)*wsize.y/2);
		}
		parent.fill(255,255);
		parent.textSize(sz*.5f);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		if (cc!=null) {
			if (cc.cc1!=-1)
				parent.text(String.format("CC%d=%.0f",cc.cc1,cc1val),x,y-sz*0.3f);
			if (cc.cc2!=-1)
				parent.text(String.format("CC%d=%.0f",cc.cc2,cc2val),x,y+sz*0.3f);
		}
	}

	void connect(Generator parent) {
		if (parent.nControllers >= parent.genType.cclist.length) {
			PApplet.println("Not connecting additional controller to "+parent.id+", which already has "+parent.nControllers+" controllers");
			return;
		}
		this.parent=parent;
		this.cc=parent.genType.cclist[parent.nControllers];
		PApplet.println("Connect controller "+id+" to generator "+parent.id+" on controllers "+this.cc.cc1+","+this.cc.cc2);
		parent.nControllers++;
	}

	void disconnect() {
		PApplet.println("Disconnect controller "+id+" from generator "+parent.id);
		parent.nControllers--;
		parent=null;
	}
}

class Fiducials extends HashMap<Integer,Fiducial> {
	static final float DISTBREAK=0.5f;   // Distance to break connections (in screen normalized coordinates)
	static final float DISTCREATE=0.1f;  // Distance to create connections 
	private static final long serialVersionUID = -1131006311643745996L;

	Fiducials() {
		super();
	}


	void addNew(Position pos, int id) {
		int ngen=0, nctrl=0;
		for (Fiducial f: values()) {
			if (f instanceof Generator)
				ngen++;
			else
				nctrl++;
		}
		if (ngen==0 || nctrl>ngen)
			put(id, new Generator(pos,Generator.genTypes[ngen%Generator.NUMGENTYPES]));
		else
			put(id, new Controller(pos));
	}
	
	void remove(int id) {
		for (Fiducial f: values()) {
			if (f.id==id) {
				f.stop();
				if (f instanceof Controller) {
					Controller c=(Controller)f;
					if (c.parent==f)
						c.disconnect();
				}
			}	
		}
		super.remove(id);
	}

	/* Make links from fiducials to lower numbered ones */
	void makeLinks() {
		for (Fiducial f: values()) {
			if (f instanceof Controller) {
				Controller c=(Controller)f;
				if (c.parent != null) {
					// Check if we need to break connection
					float dist=PVector.dist(c.origin,c.parent.origin);
					if (dist>DISTBREAK) {
						c.disconnect();
					}
				}
				if (c.parent==null) {
					// See if we can form a new connection
					float mindist=1e10f;
					Generator newparent=null;
					for (Fiducial f2: values()) {
						if (f2 instanceof Generator) {
							float dist=PVector.dist(c.origin, f2.origin);
							//PApplet.println("Distance from "+f.id+" to "+f2.id+" = "+dist);
							if (dist<mindist) {
								mindist=dist;
								newparent=(Generator)f2;
							}
						}
					}
					if (newparent!=null && mindist<=DISTCREATE) {
						c.connect(newparent);
					}
				}
			}
		}
	}

	void draw(PApplet parent, PVector wsize) {
		float sz=30;
		for (Fiducial f: values()) {
			int c=f.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			parent.strokeWeight(5);
			f.draw(parent,wsize,sz);
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
				fiducials.addNew(allpos.get(id),id);
			}

			Fiducial f=fiducials.get(id);
			f.update(allpos.get(id));
		}
		// Remove fiducials for which we no longer have a position (exitted)
		boolean done;
		do {
			done=true;
			for (int id: fiducials.keySet()) {
				if (!allpos.positions.containsKey(id)) {
					PApplet.println("Removing ID "+id);
					fiducials.remove(id);
					done=false;
					break;  // Avoid concurrent modification problem
				}
			}
		} while (!done);
		fiducials.makeLinks();
	}

	@Override
	public void start() {
		OscMessage msg = new OscMessage("/chuck/start");
		Tracker.sendOSC("CK",msg);
	}

	@Override
	public void stop() {
		OscMessage msg = new OscMessage("/chuck/stop");
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


