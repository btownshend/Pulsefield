package com.pulsefield.tracker;

import java.awt.Color;
import java.util.Random;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

// A simple Particle class.
class Particle {
    final static Logger logger = Logger.getLogger(Tracker.class.getName());
	
	PVector location;
	PVector velocity = new PVector(0, 0);
	PVector acceleration = new PVector(0, 0);
	float accelerationFuzz = -0.03f; // Tendency to move in a random direction.
	float xMaxFuzz = 0.0f; // Tendency to move in direction x.
	float yMaxFuzz = 0.0f; // Tendency to move in direction y.
	private int maxLifespan = 500; // In count-of-updates.
	int lifespan = maxLifespan;
	float opacity = 1.0f;
	float fadeDeath = 0.0f; // fade opacity in the last e.g. 10% (0.1f) of the
							// particle's life.
	float scale = 1.0f; // Particle specific scale.
	float forceRotation = 0.0f;

	int color = Color.WHITE.hashCode();
	float rotationRate = 0.0f;
	float rotationRadians = 0.0f;

	float maxOpacity = 1.0f;

	// Should the particle be culled if off screen by dieOffscreenBuffer meters?
	Boolean dieOffscreen = true;
	float dieOffscreenBuffer = 0.5f;

	static Random rng = new Random();

	Particle(PVector location) {
		this.location = location.copy();
	}

	Particle(PVector location, ParticleSystemSettings pss) {
		this(location = location.copy());
		loadParticleSystemSettings(pss);
	}

	void loadParticleSystemSettings(ParticleSystemSettings pss) {
		PVector acceleration = Particle.genRandomVector(pss.particleRandomDriftAccel, pss.tiltx, pss.tilty);
		this.acceleration = acceleration;
		this.fadeDeath = pss.fadeDeath;
		this.rotationRate = pss.particleRotation;
		this.setLifespan(pss.particleMaxLife);
		this.opacity = pss.startOpacity;
	}

	void setLifespan(int lifespan) {
		this.maxLifespan = lifespan;
		this.lifespan = lifespan;
	}

	// Particle PVector utilities.
	// Fuzz the input vector; used to spread and vary particle position,
	// velocity, acceleration.
	public static PVector fuzzVector(PVector vector, float xMaxFuzz, float yMaxFuzz) {
		return new PVector((float) rng.nextGaussian() * xMaxFuzz + vector.x,
				(float) rng.nextGaussian() * yMaxFuzz + vector.y);
	}

	// Generate a vector with a random direction and magnitude with a
	// directional force.
	public static PVector genRandomVector(float dispersionForce, float accelX, float accelY) {
		return fuzzVector(new PVector(accelX, accelY), dispersionForce, dispersionForce);
	}

	// Attract particle to position c with force.
	void attractor(PVector c, float force, float forceRotation) {
		PVector dir = PVector.sub(c, location);
		float dist2 = dir.magSq();
		dir.normalize();
		dir.mult(100f / 300 * force / PApplet.max(100.0f / 300, dist2));
		dir.rotate(forceRotation);
		velocity.add(dir);
	}

	// Push a particle away from position c with vector pushvel.
	void push(PVector c, PVector pushvel) {
		// Push particles with pushvel velocity
		// Scales with distance
		float distance = PVector.sub(c, location).mag();
		PVector direction = pushvel.copy();
		float releffect = PApplet.max(0.0f, (1.0f - (distance * 3f)));
		direction.mult(releffect);
		location.add(direction);
	}

	// Update particle location.
	void update() {
		velocity.add(acceleration);
		location.add(velocity);
		
		if (rotationRate != 0.0f) {
			// Increment the rotation and keep range in +/- 2Pi.
			rotationRadians += rotationRate % (Math.PI * 2);
		}
		
		// -1 maxLifespan means infinite life.
		if (maxLifespan == -1) {
			return;
		}
		
		lifespan--;
		
		// Fade Death is intended to:
		// Start fading at fadeDeath of the particle's age (e.g. if it's .2
		// (20%) and the particle
		// has a maxLifespan of 100 then fading will happen when there is 20
		// lifespan left.
		if ((lifespan / maxLifespan) <= fadeDeath) {
			opacity = opacity - (opacity / (lifespan));
		}
	}

	// Is the particle still useful?
	boolean isDead() {
		// Kill off-screen particles (if enabled).
		if (dieOffscreen) {
			if ((location.x > Tracker.maxx + dieOffscreenBuffer) || (location.x < Tracker.minx - dieOffscreenBuffer)
					|| (location.y > Tracker.maxy + dieOffscreenBuffer)
					|| (location.y < Tracker.miny - dieOffscreenBuffer))
				return true;
		}

		// Report invisible or lifeless particles as dead for the particleSystem
		// to cull.
		return (lifespan <= 0 || opacity <= 0.0);
	}

	// Display the particle; note secondary scale.
	void draw(PGraphics g, float scale) {
		if (rotationRadians != 0f) {
			rotateBegin(g, location);
			drawParticle(g, new PVector(0, 0), scale * this.scale);
			rotateEnd(g);
		} else {
			drawParticle(g, location, scale * this.scale);
		}
	}

	// TODO: There is probably a more elegant way to do this.
	// For rotation handling; always use with rotateEnd drawing the particle in
	// between.
	void rotateBegin(PGraphics g, PVector l) {
		g.pushMatrix();
		g.translate(l.x, l.y);
		g.rotate(rotationRadians);
	}

	void drawParticle(PGraphics g, PVector l, float scale) {
		// Draw the particle.
		g.fill(color, opacity * 255);
		g.noStroke();
		g.shapeMode(PConstants.CENTER);
		g.rect(l.x, l.y, 0.02f * scale, 0.02f * scale);
	}

	void rotateEnd(PGraphics g) {
		g.popMatrix();
	}
}

class TextParticle extends Particle {
	String text;

	TextParticle(PVector location, String text) {
		super(location);
		this.text = text;
	}	
	
	TextParticle(PVector location, ParticleSystemSettings pss, String text) {
		super(location, pss);
		this.text = text;
	}

	@Override
	void drawParticle(PGraphics g, PVector l, float scale) {
		// Draw the particle.
		g.fill(color, opacity * 255);
		g.textAlign(PConstants.CENTER, PConstants.CENTER);
		Visualizer.drawText(g, 0.1f * scale, text, l.x, l.y);
	}
}

class TriangleParticle extends Particle {

	TriangleParticle(PVector location) {
		super(location);
	}
	
	TriangleParticle(PVector location, ParticleSystemSettings pss) {
		super(location, pss);
	}

	@Override
	void drawParticle(PGraphics g, PVector l, float scale) {
		float sideLen = 5.0f;
		g.noStroke();
		g.shapeMode(PConstants.CENTER);
		g.fill(color, opacity * 255);
		g.triangle(l.x, l.y, l.x + (scale * sideLen / Tracker.getPixelsPerMeter()), l.y, l.x,
				l.y + (scale * sideLen / Tracker.getPixelsPerMeter()));
	}
}

class ImageParticle extends Particle {
	PImage image;

	ImageParticle(PVector location, PImage image) {
		super(location);
	}
	
	ImageParticle(PVector location, ParticleSystemSettings pss, PImage image) {
		super(location, pss);
//		if (image == null || !image.isLoaded()) {
		if (image == null) {

			logger.severe("Image " + image + " missing or not loaded in constructor.");
		}
		this.image = image;
	}

	@Override
	void drawParticle(PGraphics g, PVector l, float scale) {
		if (image == null) {
			logger.severe("Image " + image + " missing.  Not drawing.");
			return;
		}

		g.imageMode(PConstants.CENTER);
		g.tint(color, Math.min(opacity * 255, maxOpacity * 255));
		g.image(image, l.x, l.y, scale * image.width / Tracker.getPixelsPerMeter(),
				scale * image.height / Tracker.getPixelsPerMeter());
	}
}
