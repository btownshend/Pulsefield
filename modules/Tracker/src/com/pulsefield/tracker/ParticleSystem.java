package com.pulsefield.tracker;

import java.util.ArrayList;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;
import processing.opengl.PGL;

class ParticleSystemSettings {
	float particleRandomDriftAccel = 0.02f / 300;
	float forceRotation = 0.0f;
	float particleRotation = 0.0f;
	int particleMaxLife = 500;
	
	// blendMode can be -1 (for custom) or PGraphics.BLEND, .ADD, .SUBTRACT, etc.
	int blendMode = -1; 
	float personForce = 0.005f;
	float particleScale = 1.0f;

	float fadeDeath = 0.1f;
	float startOpacity = 1.0f;

	// Net force up/down/left/right.
	float tiltx = 0.0f;
	float tilty = 0.0f;

	// maxParticles is the hard upper limit on number of particles.
	long maxParticles = 50000;
}


class ParticleSystem {
	// An arraylist for all the particles.
	ArrayList<Particle> particles;
	ParticleSystemSettings settings;
	
	ParticleSystem() {
		particles = new ArrayList<Particle>(); // Initialize the arraylist.
		settings = new ParticleSystemSettings();
	}

	void attractor(PVector c, float force) {
		// Add a gravitational force that acts on all particles.
		for (int i = particles.size() - 1; i >= 0; i--) {
			particles.get(i).attractor(c, force, PApplet.radians(settings.forceRotation));
		}
	}
	
	void applyPeopleGravity(People p) {
		if (settings.personForce != 0.0f) {
			for (Person pos : p.pmap.values()) {
				attractor(pos.getOriginInMeters(), settings.personForce);
			}
		}
	}

	void push(PVector c, PVector spd) {
		for (Particle p : particles) {
			p.push(c, spd);
		}
	}

	// Update particle system.
	void update() {
		Iterator<Particle> i = particles.iterator();
		while (i.hasNext()) {
			Particle p = i.next();
			p.update();
			if (p.isDead()) {
				i.remove();
			}
		}
	}

	// How many particles are left until the max? This allows clients that want
	// to batch particle creation to know when that's acceptable. Can return a negative
	// value if there is a deficit (e.g. if maxParticles is reduced after creation).
	long particlesRemaining() {
		return settings.maxParticles - particles.size();
	}

	// Called before particles are drawn; perform setup here.
	void drawPrep(PGraphics g) {		
		if (settings.blendMode == -1) {
			customBlend(g);
		} else {
			g.blendMode(settings.blendMode);
		}
	}

	void customBlend(PGraphics g) {
		g.resetShader();
		PGL pgl=g.beginPGL();
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.ONE_MINUS_SRC_ALPHA); 
		pgl.blendEquation(PGL.FUNC_ADD);
		g.endPGL();
	}

	void draw(PGraphics g) {
		drawPrep(g);
		for (Particle p : particles) {
			p.draw(g, settings.particleScale);
		}
	}

	void addParticle(Particle p) {
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
