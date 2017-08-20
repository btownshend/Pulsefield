package com.pulsefield.tracker;
import java.awt.Color;
import java.util.Random;

import netP5.Logger;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;


// A simple Particle class.
class Particle {
	PVector location;
	PVector velocity = new PVector(0,0);
	PVector acceleration = new PVector(0,0);
	float accelerationFuzz = -0.03f; // Tendency to move in a random direction.
	float xMaxFuzz = 0.0f; // Tendency to move in direction x.
	float yMaxFuzz = 0.0f; // Tendency to move in direction y.
	int maxLifespan = 500; // In count-of-updates.
	int lifespan=maxLifespan;
	float opacity = 1.0f;
	float fadeDeath = 0.0f; // fade opacity in the last e.g. 10% (0.1f) of the particle's life.
	
	int color = Color.WHITE.hashCode();
	float rotationRate = 0.0f;
	float rotationRadians = 0.0f;
	
	float maxOpacity = 1.0f;
	
	// Should the particle be culled if off screen by dieOffscreenBuffer meters?
	Boolean dieOffscreen = true;
	float dieOffscreenBuffer = 0.5f;
	
	int blendMode = PGraphics.BLEND; // BLEND, ADD, SUBRACT, LIGHTEST, DARKEST, MULTIPLY, OVERLAY, etc.


	static Random rng = new Random();

	Particle(PVector location) {
		this.location = location.copy();
	}
	
	Particle(PVector location, ParticleSystem ps) {
		this(location);
		adoptSettingsFromParticleSystem(ps);
	}
	
	void adoptSettingsFromParticleSystem(ParticleSystem ps) {
		PVector acceleration = Particle.genRandomVector(ps.particleRandomDriftAccel, ps.tiltx, ps.tilty);
		this.acceleration = acceleration;
		this.fadeDeath = ps.fadeDeath;;
		this.rotationRate = ps.particleRotation;
		this.setLifespan(ps.particleMaxLife);
		this.blendMode = ps.blendMode;
		this.opacity = ps.startOpacity;
	}
	
	
	void setLifespan(int lifespan) {
		this.maxLifespan = lifespan;
		this.lifespan = lifespan;
	}
	
	
	// Particle PVector utilities.
	// Fuzz the input vector; used to spread and vary particle position, velocity, acceleration.
	private static PVector fuzzVector(final PVector vector, final float xMaxFuzz, final float yMaxFuzz) {
		return new PVector((float)rng.nextGaussian()*xMaxFuzz+vector.x, (float)rng.nextGaussian()*yMaxFuzz + vector.y);
	}

	// Generate a vector with a random direction and magnitude with a directional force.
	public static PVector genRandomVector(float dispersionForce, float accelX, float accelY) {
		return fuzzVector(new PVector(accelX, accelY), dispersionForce, dispersionForce);
	}

	// Attract particle to position c with force.
	void attractor(PVector c, float force, float forceRotation) {
		PVector dir=PVector.sub(c, location);
		float dist2=dir.magSq();
		dir.normalize();
		dir.mult(100f/300*force/PApplet.max(100.0f/300, dist2));
		dir.rotate(forceRotation);
		velocity.add(dir);
	}

	// Push a particle away from position c with vector pushvel.
	void push(PVector c, PVector pushvel) {
		// Push particles with pushvel velocity
		// Scales with distance
		float dist=PVector.sub(c, location).mag();
		PVector dir=pushvel.copy(); 
		float releffect = PApplet.max(0.0f,1.0f-dist*3f);
		dir.mult(releffect);
		location.add(dir);
	}

	// Update particle location.
	void update() {	
		velocity.add(acceleration);
		location.add(velocity);
		lifespan--;
		
		// Fade Death is intended to:
		// Start fading at fadeDeath of the particle's age (e.g. if it's .2 (20%) and the particle 
		// has a maxLifespan of 100 then fading will happen when there is 20 lifespan left.
		if ((lifespan / maxLifespan) <= fadeDeath) {
			opacity = opacity - (opacity / (lifespan));
		}
		
		if (rotationRate > 0.0f) {
			// Increment the rotation and keep range in +/- 2Pi.
			rotationRadians += rotationRate % (Math.PI * 2);
		}
	}
	
	// Display the particle.  In the base class this will be a small '+'.
	void draw(PGraphics g, float scale) {	
		if (rotationRadians != 0f) {
			rotateBegin(g, location);
			drawParticle(g, new PVector(0,0), scale);
			rotateEnd(g);			
		} else {
			drawParticle(g, location, scale);
		}
		g.blendMode(blendMode);
	}

	void drawParticle(PGraphics g, PVector l, float scale) {
		// Draw the particle.
		g.fill(color, opacity * 255);
		g.noStroke();
		g.rect(l.x, l.y, 0.02f * scale, 0.02f * scale);
	}
	
	// Is the particle still useful?
	boolean isDead() {
		// Kill off-screen particles (if enabled).
		if (dieOffscreen) {
			if ((location.x > Tracker.maxx + dieOffscreenBuffer) || (location.x < Tracker.minx - dieOffscreenBuffer)
					|| (location.y > Tracker.maxy + dieOffscreenBuffer) || (location.y < Tracker.miny - dieOffscreenBuffer))
				return true;
		}
		
		// Report invisible or lifeless particles as dead for the particleSystem to cull.
		return (lifespan <= 0 || opacity <= 0.0);
	}
	
	// TODO: There is probably a more elegant way to do this.
	// For rotation handling; always use with rotateEnd drawing the particle in between.
	void rotateBegin(PGraphics g, PVector l) {
			g.pushMatrix();
			g.translate(l.x, l.y);
			g.rotate(rotationRadians);
	}
	
	void rotateEnd(PGraphics g) {
	 		g.popMatrix();
	}
	
}

class TextParticle extends Particle {
	String text;
	
	TextParticle(PVector location, ParticleSystem ps, String text) {
		super(location, ps);
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
	
	TriangleParticle(PVector location, ParticleSystem ps) {
		super(location, ps);
	}
	
	@Override
	void drawParticle(PGraphics g, PVector l, float scale) {
		float sideLen = 0.2f;
		g.noStroke();
		g.fill(color, opacity * 255);
		g.triangle(l.x, l.y, l.x+(scale * sideLen/Tracker.getPixelsPerMeter()),
				l.y, l.x, l.y+(scale * sideLen/Tracker.getPixelsPerMeter()));
	}
}

class ImageParticle extends Particle {
	
	PImage image;
	
	ImageParticle(PVector location, ParticleSystem ps, PImage image) {
		super(location, ps);
		
		this.image = image;
	}
	
	@Override
	void drawParticle(PGraphics g, PVector l, float scale) {
		g.imageMode(PConstants.CENTER);
		g.tint(color,Math.min(opacity * 255, maxOpacity * 255));
		g.image(image, location.x, location.y, scale * image.width/Tracker.getPixelsPerMeter(), scale * image.height/Tracker.getPixelsPerMeter());
	}
}
