package com.pulsefield.tracker;
import java.util.HashSet;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

class Molecule {
	private static final float G=0.02f;   // Gravitational constant
	private static final float BONDLENGTH=0.2f;
	private static final float MAXSPEED=2f;   // Max speed in m/s
	private static final float DAMPING=0.9f;
	
	PVector location;
	PVector velocity;
	PImage img;
	float radius;
	boolean isAlive;
	float mass;
	float tgtSpeed;   // Target speed (temperature)
	float angle;
	float angularVelocity;
    private final static Logger logger = Logger.getLogger(Molecule.class.getName());

	private static Images imgs=null;
	
	Molecule(PVector pos, PVector vel, float radius) {
		velocity=new PVector();
		velocity.x=vel.x; velocity.y=vel.y;
		location=new PVector();
		location.x=pos.x; location.y=pos.y;
		if (imgs==null)
			imgs=new Images("freeze/molecules");
		img=imgs.getRandom();
		this.radius=radius;
		mass=1f;
		angle=0;
		angularVelocity=(float) (0.2f*(Math.random()-0.5f));
		//logger.fine("Created marble at "+location+" with velocity "+vel+" and mass "+mass);
		isAlive=true;
	}
	
	public void destroy() {
		//logger.fine("Destroy marble at "+location);
		isAlive=false;
	}
	
	public float getRadius() {
		return radius;
		//return (float) Math.pow(mass/DENSITY,1/2f);
	}
	
	public PVector getMomentum() {
		return PVector.mult(velocity, mass);
	}
	
	public void update() {
		// Damping
		float curspeed=velocity.mag();
		float newspeed=curspeed*DAMPING+tgtSpeed*(1-DAMPING);
		velocity.normalize().mult(newspeed);
		//velocity.add(PVector.mult(velocity, -DAMPING/Tracker.theTracker.frameRate));
		if (velocity.mag() > MAXSPEED) {
			logger.fine("Clipping speed from "+velocity.mag());
			velocity.normalize().mult(MAXSPEED);
		}
		location.add(PVector.mult(velocity,1/Tracker.theTracker.frameRate));
		angle= (float)((angle+angularVelocity)%(2*Math.PI));
		float radius=getRadius();
		// Out of bounds, bounce back
		if (location.x-radius > Tracker.maxx && velocity.x>0)
			destroy(); //velocity.x=-velocity.x;
		if (location.x+radius < Tracker.minx && velocity.x<0)
			destroy(); //velocity.x=-velocity.x;
		if (location.y-radius > Tracker.maxy && velocity.y>0)
			destroy(); //velocity.y=-velocity.y;
		if (location.y+radius < Tracker.miny && velocity.y<0)
			destroy(); //velocity.y=-velocity.y;
	}
	
	public void draw(PGraphics g) {
		float r=getRadius();
		//g.ellipse(location.x, location.y, r*2, r*2);
		g.imageMode(PConstants.CENTER);
		g.pushMatrix();
		g.translate(location.x, location.y);
		g.rotate(angle);
		g.image(img,0,0,r*2,r*2);
		g.popMatrix();
	}

	// Get force between two Molecules at given separation
	// Positive force is attractive
	public float getForce(float sep) {
		float force=(float)(G/(sep*sep)*(1-Math.pow(BONDLENGTH/sep,1f)));
		return force;
	}

	public void interact(Molecule b2) {
		PVector sep=PVector.sub(b2.location, location);
		float dist=sep.mag();
		float force=getForce(dist);  // positive force is attractive
		PVector accel = PVector.mult(sep, mass*force/dist);
		//logger.fine("sep="+sep+", dist="+dist+", force="+force+", vel="+b1.velocity+", accel="+accel);
		velocity.add(PVector.mult(accel,1/Tracker.theTracker.frameRate));
	}
}

class Butterfly extends Molecule {
	private static Images imgs=null;
	
	Butterfly(PVector pos, PVector vel, float radius) {
		super(pos,vel,radius);
		if (imgs==null)
			imgs=new Images("freeze/butterflies");
		img=imgs.getRandom();
		angularVelocity=0;
		angle=(float)Math.atan2(vel.x,-vel.y);
	}
	public void draw(PGraphics g) {
		float r=getRadius();
		//g.ellipse(location.x, location.y, r*2, r*2);
		g.imageMode(PConstants.CENTER);
		g.pushMatrix();
		g.translate(location.x, location.y);
		g.rotate(angle);
		g.image(img,0,0,r*2,r*2);
		g.popMatrix();
	}
}

class Strut extends Molecule {
	float length;
	
	Strut(PVector pos, PVector vel, float radius) {
		super(pos,vel,radius);
		length=0.1f;
	}
	
	public void draw(PGraphics g) {
		//g.ellipse(location.x, location.y, r*2, r*2);
		g.imageMode(PConstants.CENTER);
		g.pushMatrix();
		g.translate(location.x, location.y);
		g.rotate(angle);
		g.stroke(0xffff0000);
		g.line(-length/2, 0f, length/2, 0f);
		g.popMatrix();
	}
}

class MoleculeFactory {
	private static final float INITRADIUS=0.1f;
	private static final float CREATERATE=0.5f;  // Average number created per frame
	private static final float CREATESPEED=0.02f;  // Create speed in m/s
	private static HashSet<Molecule> allMolecules = new HashSet<Molecule>();
	private String moleculeType;
	static String[] moleculeTypes={"MOLECULE","STRUT","BUTTERFLY"};
	
	MoleculeFactory(String molType) {
		moleculeType = molType;
	}
	public Molecule create(PVector pos, PVector vel, float radius)  {
		Molecule o=null;
		if (moleculeType.equalsIgnoreCase("MOLECULE"))
			o=new Molecule(pos,vel,radius);
		else if (moleculeType.equalsIgnoreCase("STRUT"))
			o=new Strut(pos,vel,radius);
		else if (moleculeType.equalsIgnoreCase("BUTTERFLY"))
			o=new Butterfly(pos,vel,radius);
		else 
			assert(false);
		allMolecules.add(o);
		return o;
	}
	
	// Create a new Molecule somewhere on wall
	public Molecule createOnWall() {
		float pos=(float)(Math.random()*(Tracker.getFloorSize().x+Tracker.getFloorSize().y)*2);
		PVector loc=new PVector(0,0);
		float radius=INITRADIUS;
		if (pos<Tracker.getFloorSize().x) {
			loc.x=pos;
			loc.y=-Tracker.getFloorSize().y/2-radius;
		} else if (pos<Tracker.getFloorSize().x*2) {
			loc.x=pos-Tracker.getFloorSize().x;
			loc.y=Tracker.getFloorSize().y/2+radius;
		} else if (pos<Tracker.getFloorSize().x*2+Tracker.getFloorSize().y) {
			loc.x=-Tracker.getFloorSize().x/2-radius;
			loc.y=pos-Tracker.getFloorSize().x*2;
		} else {
			loc.x=Tracker.getFloorSize().x/2+radius;
			loc.y=pos-Tracker.getFloorSize().x*2-Tracker.getFloorSize().y;
		}
		loc.add(Tracker.getFloorCenter());
		PVector vel=PVector.mult(PVector.random2D(),CREATESPEED);
		return create(loc,vel,radius);
	}
	
	public void updateAll(People allpos, Effects effects) {
		// Remove dead molecules
		for (Molecule o: allMolecules) {
			if (!o.isAlive)
				allMolecules.remove(this);
		}
		// Create new Molecules
		for (int i=0;i<CREATERATE;i++) {
			if (i+1<CREATERATE || Math.random()<CREATERATE-i)
				createOnWall();
		}
		Object all[]=allMolecules.toArray();
		for (Object o: all) {
			Molecule b=(Molecule)o;
			float temp=VisualizerFreeze.getTemperature(allpos, b.location);
			b.tgtSpeed=temp/300f;
			b.update();
		}
		// Find all contacts
		for (Object o1: all) {
			Molecule b1=(Molecule)o1;
			if (!b1.isAlive)
				continue;
			for (Object o2: all) {
				Molecule b2=(Molecule)o2;
				if (!b2.isAlive)
					continue;
				if (b1==b2)
					continue;
				b1.interact(b2);   // Modify b1 based on interaction with b2
			}
		}
	}
	public void drawAll(PGraphics g) {
		for (Molecule b: allMolecules) {
			b.draw(g);
		}
	}
	public void destroyAll() {
		Object all[]=allMolecules.toArray();
		for (Object o: all) {
			Molecule b=(Molecule)o;
			b.destroy();
		}
	}
}

// Freeze visualizer
public class VisualizerFreeze extends Visualizer {
	long startTime;
	Effects effects;
	MoleculeFactory mFactory;

	VisualizerFreeze(PApplet parent, Synth synth) {
		super();
		effects=Effects.defaultEffects;
	}
	
	public void update(PApplet parent, People allpos) {		
		// Update internal state of the Molecules
		mFactory.updateAll(allpos, effects);
	}


	public void start() {
		super.start();
		String mType=MoleculeFactory.moleculeTypes[(int)(Math.random()*MoleculeFactory.moleculeTypes.length)];
		logger.info("Starting Freeze with molecule type "+mType);
		mFactory=new MoleculeFactory(mType);
		Ableton.getInstance().setTrackSet("Freeze");
	}

	public void stop() {
		mFactory.destroyAll();
		super.stop();
		logger.info("Stopping Freeze at "+System.currentTimeMillis());
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g,p);
		if (false) {
		g.loadPixels();
		PImage buffer=null;
		int downSample=4;
		buffer = new PImage(g.width/downSample, g.height/downSample);

		for (int y = 0; y < buffer.height; y++) {
			float ypos=Tracker.miny+(Tracker.maxy-Tracker.miny)*y/buffer.height;
			for (int x = 0; x < buffer.width; x++) {
				float xpos=Tracker.minx+(Tracker.maxx-Tracker.minx)*x/buffer.width;
				float temp=getTemperature(p,new PVector(xpos,ypos));
				buffer.set(x, y, (int)((300f-temp)/300f*256));
			}
		}
		g.imageMode(PConstants.CENTER);
		g.tint(255);  // Causes slow fade if <255
		g.image(buffer,Tracker.getFloorCenter().x,Tracker.getFloorCenter().y,g.width/Tracker.getPixelsPerMeter(),g.height/Tracker.getPixelsPerMeter());
		}
		mFactory.drawAll(g);
	}

	// Get the temperature at the given position in Kelvin
	public static float getTemperature(People p, PVector pos) {
		float totalWt=0f;
		float totalTemp=0f;
		totalWt+=1.0f;   // Background temperature of 300K
		totalTemp+=totalWt*300f;
		for (Person ps: p.pmap.values()) {  
			float d=PVector.dist(ps.getOriginInMeters(),pos);
			float wt=1f/(d*d);
			totalWt+=wt;
			totalTemp+=wt*getTemperature(ps);
		}
		return totalTemp/totalWt;
	}

	private static float getTemperature(Person p) {
		return 300f*Math.min(1.0f,p.getVelocityInMeters().mag()/0.5f);
	}

}

