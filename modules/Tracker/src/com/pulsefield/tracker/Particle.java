package com.pulsefield.tracker;
import java.util.Random;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

// A simple Particle class

class Particle {
	static Random rng = new Random();
	PVector location;
	PVector velocity;
	PVector acceleration;
	float attractorRotation;
	float lifespan, maxlifespan;
	int color;
	PImage img;

	Particle(PVector loc, PVector velo, int color, PImage img) {
		// Note: Default set here:
		// attractorRotation specified here as 90 degrees.
		// maxlifespan as 500.0.

		this(loc, getDefaultVector(velo), getDefaultAcceleration(), 90, 500.0f, color, img);
	}

	private static PVector getDefaultVector(PVector velo) {
		return fuzzVector(velo, 0.3f/300, 0.3f/300);
	}

	private static PVector getDefaultAcceleration() {
		final PVector defaultAccel = new PVector(0f, -0.03f/300);
		return defaultAccel;
	}

	// Fuzz the input vector; used to spread and vary particle velocity and acceleration.
	private static PVector fuzzVector(PVector vector, float xMaxFuzz, float yMaxFuzz) {
		return new PVector((float)rng.nextGaussian()*xMaxFuzz+vector.x, (float)rng.nextGaussian()*yMaxFuzz + vector.y);
	}

	Particle(PVector loc, PVector velo, PVector accel, float rotation, float maxlife, int color, PImage img) {
		this.img=img;
		this.color=color;
		location = loc.copy();
		velocity = velo.copy();
		acceleration = accel;
		attractorRotation = rotation;
		maxlifespan = maxlife;
		lifespan = maxlifespan;
	}

	// Utility function to generate a vector with a random direction and magniture plus a directional force.
	public static PVector genRandomVector(float dispersionForce, float accelVectorX, float accelVectorY) {
		double maxRange = dispersionForce;
		double minRange = -1 * dispersionForce;
		float xaccel = (float) (minRange + Math.random() * (maxRange-minRange)) + accelVectorX;
		float yaccel = (float) (minRange + Math.random() * (maxRange-minRange)) + accelVectorY;
		return new PVector(xaccel,yaccel);
	}

	void attractor(PVector c, float force) {
		// Attraction to c
		PVector dir=PVector.sub(c, location);
		float dist2=dir.magSq();
		dir.normalize();
		dir.mult(100f/300*force/PApplet.max(100.0f/300, dist2));
		dir.rotate(attractorRotation);
		velocity.add(dir);
	}

	void push(PVector c, PVector pushvel) {
		// Push particles with pushvel velocity
		// Scales with distance
		float dist=PVector.sub(c, location).mag();
		PVector dir=pushvel.copy(); 
		float releffect = PApplet.max(0.0f,1.0f-dist*3f);
		dir.mult(releffect);
		location.add(dir);
	}

	// Method to update location
	void update() {
		velocity.add(acceleration);

		location.add(velocity);
		lifespan -= 1;
		//acceleration.x=0;acceleration.y=0;
		//println(""+id+location+lifespan);
	}

	// Method to display
	void draw(PGraphics g) {
		//println("display(): location="+location);
		g.imageMode(PConstants.CENTER);
		//float kscale= 0.05f;//lifespan/maxlifespan/10;
		float kscale=(float)Math.pow(1.0-lifespan/(2*maxlifespan),0.1)/100+0.05f;
		kscale=0.05f;
		g.tint(color,(int)(kscale*255.0));
		g.image(img, location.x, location.y, img.width/Tracker.getPixelsPerMeter(),img.height/Tracker.getPixelsPerMeter());

		//stroke(col, lifespan);
		//fill(col, lifespan);
		//ellipse(location.x, location.y, 8, 8);
	}

	// Is the particle still useful?
	boolean isDead() {
		//if (lifespan<0.0)
		//println("Particle at "+location+" is dead");

		return (lifespan < 0.0);
	}
}
