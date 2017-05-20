package com.pulsefield.tracker;
import java.util.HashSet;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PMatrix;
import processing.core.PMatrix2D;
import processing.core.PVector;

class Molecule {
	private static final float G=0.02f;   // Gravitational constant
	private static final float INITRADIUS=0.1f;
	private static final float BONDLENGTH=2*INITRADIUS;
	private static final float MAXEFFECTDIST=BONDLENGTH*10;   // Maximum distance for which we compute interactions
	private static final float CREATERATE=0.5f;  // Average number created per frame
	private static final float CREATESPEED=0.02f;  // Create speed in m/s
	private static final float MAXSPEED=2f;   // Max speed in m/s
	private static final float DAMPING=0.9f;
	
	PVector location;
	PVector velocity;
	PImage img;
	float radius;
	boolean isAlive;
	float mass;
	float tgtSpeed;   // Target speed (temperature)
	
	private static Images imgs=null;
	private static HashSet<Molecule> allMolecules = new HashSet<Molecule>();
	
	Molecule(PVector pos, PVector vel, PImage img, float radius) {
		velocity=new PVector();
		velocity.x=vel.x; velocity.y=vel.y;
		location=new PVector();
		location.x=pos.x; location.y=pos.y;
		assert(img!=null);
		this.img=img;
		this.radius=radius;
		mass=1f;
		allMolecules.add(this);
		//PApplet.println("Created marble at "+location+" with velocity "+vel+" and mass "+mass);
		isAlive=true;
	}
	public void destroy() {
		//PApplet.println("Destroy marble at "+location);
		allMolecules.remove(this);
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
			PApplet.println("Clipping speed from "+velocity.mag());
			velocity.normalize().mult(MAXSPEED);
		}
		location.add(PVector.mult(velocity,1/Tracker.theTracker.frameRate));
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
		g.image(img,location.x,location.y,r*2,r*2);
	}
	static Molecule create(PVector pos, PVector vel, float radius)  {
		if (imgs==null)
			imgs=new Images("freeze/molecules");
		return new Molecule(pos,vel,imgs.getRandom(),radius);
	}
	// Create a new Molecule somewhere on wall
	static Molecule createOnWall() {
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
	static void destroyAll() {
		Object all[]=allMolecules.toArray();
		for (Object o: all) {
			Molecule b=(Molecule)o;
			b.destroy();
		}
	}
	// Get force between two Molecules at given separation
	// Positive force is attractive
	static float getForce(float sep) {
		float force=(float)(G/(sep*sep)*(1-Math.pow(BONDLENGTH/sep,1f)));
		return force;
	}
	static void updateAll(People allpos, Effects effects) {
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
				PVector sep=PVector.sub(b2.location, b1.location);
				float dist=sep.mag();
				if (dist > MAXEFFECTDIST)
					continue;
				float force=getForce(dist);  // positive force is attractive
				PVector accel = PVector.mult(sep, b1.mass*force/dist);
				//PApplet.println("sep="+sep+", dist="+dist+", force="+force+", vel="+b1.velocity+", accel="+accel);
				b1.velocity.add(PVector.mult(accel,1/Tracker.theTracker.frameRate));
			}
		}
	}
	static void drawAll(PGraphics g) {
		for (Molecule b: allMolecules) {
			b.draw(g);
		}
	}
}

// Osmos visualizer
public class VisualizerFreeze extends Visualizer {
	long startTime;
	Images MoleculeImages;
	Effects effects;

	VisualizerFreeze(PApplet parent, Synth synth) {
		super();
		MoleculeImages=new Images("freeze/Molecules");
		effects=new Effects(synth,123);
		effects.add("COLLIDE",52,55);
		effects.add("SPLIT",40,42);
	}
	
	public void update(PApplet parent, People allpos) {		
		// Update internal state of the Molecules
		Molecule.updateAll(allpos, effects);
	}


	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Freeze");
	}

	public void stop() {
		Molecule.destroyAll();
		super.stop();
		PApplet.println("Stopping Freeze at "+System.currentTimeMillis());
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g,p);
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
		
		Molecule.drawAll(g);
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

