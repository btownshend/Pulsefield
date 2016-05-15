import java.util.ArrayList;
import java.util.HashMap;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

// Visualizer that sends messages to chuck
abstract class Fiducial extends Person {
	Fiducial(Person ps) {
		super(ps.getOriginInMeters(),ps.channel,ps.id);
	}

	void update(Person ps) {
		setNormalizedPosition(ps.getNormalizedPosition());
	}

	abstract void draw(PGraphics parent, PVector wsize, float sz);
	void stop() { }
}

class CCPair {
	int cc1,cc2;
	String nm1,nm2;
	PVector rng1, rng2;
	int color;
	CCPair() {
		cc1=-1;
		cc2=-1;
	}
	CCPair(int cc1, String nm1,PVector rng1, int cc2, String nm2,PVector rng2, int color) {
		this.cc1=cc1;
		this.cc2=cc2;
		this.nm1=nm1;
		this.nm2=nm2;
		this.color=color;
		this.rng1=rng1;
		this.rng2=rng2;
	}
	CCPair(int cc1, String nm1, int cc2, String nm2, int color) {
		this.cc1=cc1;
		this.cc2=cc2;
		this.nm1=nm1;
		this.nm2=nm2;
		this.rng1=new PVector(0f,128.0f);
		this.rng2=new PVector(0f,128.0f);
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
		cclist[1]=new CCPair(-1,"",-1,"",0);
	}
	GeneratorType(String name, int color, int code, CCPair cc1, CCPair cc2) {
		this.name=name;
		this.color=color;
		this.code=code;
		this.cclist=new CCPair[2];
		cclist[0]=cc1;
		cclist[1]=cc2;
	}
	GeneratorType(String name, int color, int code, CCPair cc1, CCPair cc2, CCPair cc3) {
		this.name=name;
		this.color=color;
		this.code=code;
		this.cclist=new CCPair[3];
		cclist[0]=cc1;
		cclist[1]=cc2;
		cclist[2]=cc3;
	}
	GeneratorType(String name, int color, int code, CCPair cc1, CCPair cc2, CCPair cc3, CCPair cc4) {
		this.name=name;
		this.color=color;
		this.code=code;
		this.cclist=new CCPair[4];
		cclist[0]=cc1;
		cclist[1]=cc2;
		cclist[2]=cc3;
		cclist[3]=cc4;
	}
}


class Generator extends Fiducial {
	static final CCPair cc1_11=new CCPair(1,"LFODepth",11,"LFOSpd",0xffff00ff);
	static final CCPair ccpat=new CCPair(200,"Pattern",new PVector(0.0f,15.99f),-1,"-",new PVector(0.0f,0.0f),0xff007f4f);
	static final CCPair reverb=new CCPair(201,"Reverb",new PVector(0.0f,100f),-1,"-",new PVector(0.0f,0.0f),0xff8000ff);
	static final GeneratorType genTypes[] = {
		new GeneratorType("FM",0xff0000ff,0,new CCPair(1,"LFODepth",new PVector(0.0f,500.0f),11,"LFOSpd",new PVector(1.0f,500.0f),0xffff00ff),
					ccpat,reverb),
		new GeneratorType("BeeThree",0xffff0000,1,new CCPair(2,"Feedback",4,"Op3Gain",0xffffff00),
					ccpat, cc1_11,reverb),
		new GeneratorType("ModalBar",0xff00ff00,2,new CCPair(16,"Preset",new PVector(0.0f,8.99f),1,"Mix",new PVector(0.0f,128.0f),0xffffff00),
					ccpat,new CCPair(2,"Hardness",4,"Position",0xffff00ff),
					reverb),
		new GeneratorType("FMVoices",0xff7f7f40,3,new CCPair(2,"Vowel",4,"Tilt",0xff7fff80),
					ccpat,reverb,cc1_11),
		new GeneratorType("TubeBell",0xff1f7f80,4,new CCPair(2,"ModIndex",4,"Crossfade",0xff7fff80),
					ccpat,reverb,cc1_11),
		new GeneratorType("Wurley",0xffff1f00,5,new CCPair(2,"ModIndex",4,"Crossfade",0xff7fff80),
					ccpat,reverb,cc1_11),
		new GeneratorType("Moog",0xff217fff,6,new CCPair(2,"Q",4,"SweepRate",0xff7f7f80),
					ccpat,reverb,cc1_11),
		new GeneratorType("Clarinet",0xff3fef50,7,new CCPair(2,"ReedStiff",4,"NGain",0xff7ffff0),
					ccpat,reverb,cc1_11),
		new GeneratorType("Shakers",0xff3fef50,8,new CCPair(2,"Energy",4,"Decay",0xff1fff80),
				new CCPair(1071,"Preset",new PVector(0.0f,22.99f),11,"NObjects",new PVector(1.0f,10.99f),0xff1f3f30),
					ccpat,reverb),
		new GeneratorType("VoicForm",0xff1fefa0,9,new CCPair(4,"Vowel",new PVector(0f,31.99f),2,"Voicing",new PVector(0.0f,128.0f),0xff7fff80),
				ccpat,reverb,cc1_11),
					


	};

	static final int NUMGENTYPES=genTypes.length;

	GeneratorType genType;
	ArrayList<Controller> children;

	Generator(Person ps, GeneratorType genType) {
		super(ps);
		this.genType=genType;
		OscMessage msg = new OscMessage("/chuck/new");
		msg.add(id);
		msg.add(genType.code);
		Tracker.sendOSC("CK",msg);
		children=new ArrayList<Controller>();
	}

	@Override() 
	void stop() {
		OscMessage msg = new OscMessage("/chuck/del");
		msg.add(id);
		Tracker.sendOSC("CK",msg);
		PApplet.println("Stopped generator ID "+id);
		while (children.size()>0)
			children.get(0).disconnect();
	}

	@Override
	void update(Person ps) {
		super.update(ps);
		OscMessage msg = new OscMessage("/chuck/dev/"+id+"/pan");
		msg.add((ps.getNormalizedPosition().x+1f)/2f);
		Tracker.sendOSC("CK",msg);
		msg = new OscMessage("/chuck/dev/"+id+"/y");
		msg.add((-ps.getNormalizedPosition().y+1f)/2f); // Flip direction so top of screen (far side of PF) is 1.0
		Tracker.sendOSC("CK",msg);
	}

	@Override 
	void draw(PGraphics parent, PVector wsize, float sz) {
		float x=(getNormalizedPosition().x+1)*wsize.x/2;
		float y=(getNormalizedPosition().y+1)*wsize.y/2;
		parent.fill(genType.color);
		parent.stroke(genType.color);
		parent.ellipseMode(PConstants.CENTER);
		parent.ellipse(x, y, sz, sz);
		parent.fill(255);
		parent.textSize(sz*.7f);
		parent.textAlign(PConstants.CENTER,PConstants.BOTTOM);
		parent.text(genType.name,x,y-sz/2);
	}
}

class Controller extends Fiducial {
	Generator parent;
	CCPair cc;   // Controller numbers to use for (dir,dist), -1 to ignore
	float cc1val, cc2val;

	Controller(Person ps) {
		super(ps);
		parent=null;
		cc=null;
	}

	@Override
	void update(Person ps) {
		super.update(ps);
		if (parent!=null) {
			float dir=PVector.sub(parent.getNormalizedPosition(),getNormalizedPosition()).heading();
			float dist=PVector.dist(parent.getNormalizedPosition(),getNormalizedPosition());
			if (cc.cc1!=-1) {
				OscMessage msg = new OscMessage("/chuck/dev/"+parent.id+"/cc");
				msg.add(cc.cc1);
				cc1val = (float) (dir/(2*Math.PI)-0.25f);  // Map so up on screen is 0.0 and increases CW
				if (cc1val<0)cc1val=cc1val+1f;
				cc1val=cc1val*(cc.rng1.y-cc.rng1.x)+cc.rng1.x;
				msg.add(cc1val);
				Tracker.sendOSC("CK",msg);
			}
			if (cc.cc2!=-1) {
				OscMessage msg = new OscMessage("/chuck/dev/"+parent.id+"/cc");
				msg.add(cc.cc2);
				// Linear from 0.0 at DISTBREAK to 1.0 at distance DISTCREATE
				cc2val=(1.0f-(dist-Fiducials.DISTCREATE)/(Fiducials.DISTBREAK-Fiducials.DISTCREATE));
				if (cc2val>1.0f) cc2val=1.0f;
				if (cc2val<0.0f) cc2val=0.0f;
				cc2val=cc2val*(cc.rng2.y-cc.rng2.x)+cc.rng2.x;
				msg.add(cc2val);  
				Tracker.sendOSC("CK",msg);
			}
		}
	}

	@Override 
	void draw(PGraphics parent, PVector wsize, float sz) {
		float x=(getNormalizedPosition().x+1)*wsize.x/2;
		float y=(getNormalizedPosition().y+1)*wsize.y/2;
		if (cc!=null) {
			parent.fill(cc.color);
			parent.stroke(cc.color);
		}
		if (this.parent!=null) {
			parent.line((getNormalizedPosition().x+1)*wsize.x/2, (getNormalizedPosition().y+1)*wsize.y/2, (this.parent.getNormalizedPosition().x+1)*wsize.x/2, (this.parent.getNormalizedPosition().y+1)*wsize.y/2);
		}
		if (cc != null && cc.cc1==200)
			// Use triangles for patterns
			parent.triangle(x-sz/2,y+sz/2,x+sz/2,y+sz/2,x,y-sz/2);
		else {
			parent.rectMode(PConstants.CENTER);
			parent.rect(x, y, sz, sz);
		}
		parent.fill(255);
		parent.textSize(sz*.5f);
		if (cc!=null) {
			if (cc.cc1!=-1) {
				parent.textAlign(PConstants.CENTER,PConstants.BOTTOM);
				parent.text(String.format("%s=%d",cc.nm1,(int)cc1val),x,y-sz/2);
			}
			if (cc.cc2!=-1) {
				parent.textAlign(PConstants.CENTER,PConstants.TOP);
				parent.text(String.format("%s=%d",cc.nm2,(int)cc2val),x,y+sz/2);
			}
		}
	}

	void connect(Generator parent) {
		if (parent.children.size() >= parent.genType.cclist.length) {
			PApplet.println("Not connecting additional controller to "+parent.id+", which already has "+parent.children.size()+" controllers");
			return;
		}
		this.parent=parent;
		// Find a controller not being used
		int newcc;
		while (true) {
			newcc = (new java.util.Random()).nextInt(parent.genType.cclist.length);
			PApplet.println("Trying newcc="+newcc+": "+parent.genType.cclist[newcc].cc1+","+parent.genType.cclist[newcc].cc2);
			boolean inuse=false;
			for (int i=0;i<parent.children.size();i++) {
				PApplet.println("parent.children["+i+"] = "+parent.children.get(i).cc.cc1+","+parent.children.get(i).cc.cc2);
				if (parent.children.get(i).cc == parent.genType.cclist[newcc]) {
					PApplet.println("inuse");
					inuse=true;
					break;
				}
			}
			PApplet.println("newcc="+newcc+" inuse:"+inuse);
			if (!inuse)
				break;
		}
		this.cc=parent.genType.cclist[newcc];
		parent.children.add(this);
		PApplet.println("Connect controller "+id+" to generator "+parent.id+" on controllers "+this.cc.cc1+","+this.cc.cc2);
	}

	void disconnect() {
		PApplet.println("Disconnect controller "+id+" from generator "+parent.id);
		// Remove from parent list
		parent.children.remove(this);
		parent=null;
		cc=null;
	}
}

class Fiducials extends HashMap<Integer,Fiducial> {
	static final float DISTBREAK=0.5f;   // Distance to break connections (in screen normalized coordinates)
	static final float DISTCREATE=0.2f;  // Distance to create connections 
	private static final long serialVersionUID = -1131006311643745996L;

	Fiducials() {
		super();
	}

	void removeAll() {
		for (Fiducial f: values()) {
			PApplet.println("removeall: removing id "+f.id);
			f.stop();
		}
		super.clear();
	}

	void addNew(Person pos, int id) {
		int ngen=0, nctrl=0;
		for (Fiducial f: values()) {
			if (f instanceof Generator)
				ngen++;
			else
				nctrl++;
		}
		if (ngen==0 || (nctrl>0 && (new java.util.Random()).nextFloat() > 1.0f*ngen/(ngen+nctrl))) {
			int genType=(new java.util.Random()).nextInt(Generator.NUMGENTYPES);
			put(id, new Generator(pos,Generator.genTypes[genType]));
		} else
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
					float dist=PVector.dist(c.getNormalizedPosition(),c.parent.getNormalizedPosition());
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
							float dist=PVector.dist(c.getNormalizedPosition(), f2.getNormalizedPosition());
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

	void draw(PGraphics parent, PVector wsize) {
		float sz=30;
		// Draw all the controllers first so generators will be on top of connecting lines
		for (Fiducial f: values()) {
			if (f instanceof Controller) {
				int c=f.getcolor(parent);
				parent.fill(c,255);
				parent.stroke(c,255);
				parent.strokeWeight(5);
				f.draw(parent,wsize,sz);
			}
		}
		for (Fiducial f: values()) {
			if (f instanceof Generator) {
				int c=f.getcolor(parent);
				parent.fill(c,255);
				parent.stroke(c,255);
				parent.strokeWeight(5);
				f.draw(parent,wsize,sz);
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
	public void update(PApplet parent, People allpos) {
		// Update internal state of the dancers
		for (int id: allpos.pmap.keySet()) {
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
				if (!allpos.pmap.containsKey(id)) {
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
		super.start();
		OscMessage msg = new OscMessage("/chuck/start");
		Tracker.sendOSC("CK",msg);
	}

	@Override
	public void stop() {
		super.stop();
		fiducials.removeAll();
		OscMessage msg = new OscMessage("/chuck/stop");
		Tracker.sendOSC("CK",msg);
	}

	@Override
	public void draw(PGraphics parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		fiducials.draw(parent,wsize);
	}
}



