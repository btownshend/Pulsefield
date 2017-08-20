package com.pulsefield.tracker;
import java.awt.Color;
import java.util.ArrayList;
import java.util.Iterator;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

import java.util.logging.Logger;

// An ArrayList is used to manage the list of Particles

class ParticleSystem {
	// An arraylist for all the particles.
	ArrayList<Particle> particles; 
	
	// Particles can be images or circles.  If circleSize is set it overrides the image.
	PImage img;
	
	float particleRandomDriftAccel = 0.04f / 300;
	float forceRotation = 0.0f;
	float particleRotation = 0.0f;
	int particleMaxLife = 500;
	int blendMode = PGraphics.BLEND;
	float backgroundBrightness = 0.0f;
	float fadeDeath = 0.1f;
	float startOpacity = 0.95f;
	
	// Net force up/down/left/right.
	float tiltx = 0.0f;
	float tilty = 0.0f;

	
	// if maxParticles is the hard upper limit on number of particles.
	// New particles will be rejected if it's surpassed.
	long maxParticles = 50000;
	
	ParticleSystem() {
		particles = new ArrayList<Particle>();   // Initialize the arraylist
	}	
	
	ParticleSystem(PImage img) {
		particles = new ArrayList<Particle>();   // Initialize the arraylist
		this.img=img;
	}
	
	ParticleSystem(float circleSize) {
		this(new PImage());
	}

	void attractor(PVector c, float force) {
		// Add a gravitational force that acts on all particles.
		for (int i = particles.size()-1; i >= 0; i--) {
			particles.get(i).attractor(c, force, PApplet.radians(forceRotation));
		}
	}

	void push(PVector c, PVector spd) {
		// push particles we're moving through
		for (int i = particles.size()-1; i >= 0; i--) {
			particles.get(i).push(c, spd);
		}
	}

	// Update particle system.
	void update() {
		Iterator<Particle> i = particles.iterator();
		while (i.hasNext()) {
			Particle p=i.next();
			p.update();
			if (p.isDead()) {
				i.remove();
			}
		}
	}

	// How many particles are left until the max?  This allows clients that want to batch
	// particle creation to know when that's acceptable.  Can return a negative value if
	// there is a deficit (e.g. if maxParticles is reduced after creation).
	long particlesRemaining() {
		return maxParticles - particles.size();
	}
	
	void draw(PGraphics g, float particleScale) {
		g.background(backgroundBrightness * 255);
		// PApplet.println("drawing " + particles.size() + " particles of max " + maxParticles);
		for (Particle p: particles) {
			p.draw(g, particleScale);
		}
	}
	
	void addParticle(Particle p) {		
		// PApplet.println("Request to add particle (" + particlesRemaining() + "left)  Particle : " + p + " with location " + p.location);

		// Don't allow creation of more than the allowed number of particles.
		if (particlesRemaining() <= 0) {
			return;
		}

		particles.add(p);
	}

	// A method to test if the particle system still has particles
	boolean dead() {
		return particles.isEmpty();
	}
}
