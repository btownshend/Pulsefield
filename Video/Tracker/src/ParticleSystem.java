import java.util.ArrayList;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PImage;
import processing.core.PVector;


// An ArrayList is used to manage the list of Particles

class ParticleSystem {
	ArrayList<Particle> particles;    // An arraylist for all the particles
	PImage img;

	ParticleSystem(PImage img) {
		particles = new ArrayList<Particle>();   // Initialize the arraylist
		this.img=img;
	}


	void attractor(PVector c, float force) {
		// Add a gravitional force that acts on all particles
		for (int i = particles.size()-1; i >= 0; i--) {
			particles.get(i).attractor(c, force);
		}
	}

	void push(PVector c, PVector spd) {
		// push particles we're moving through
		for (int i = particles.size()-1; i >= 0; i--) {
			particles.get(i).push(c, spd);
		}
	}

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

	void draw(PApplet parent, PVector wsize) {
		for (Particle p: particles) {
			p.draw(parent, wsize);
		}
	}

	void addParticle(PVector origin, int color) {
		Particle p = new Particle(origin, new PVector(0f,0f), color, img);
		particles.add(p);
	}

	// A method to test if the particle system still has particles
	boolean dead() {
		return particles.isEmpty();
	}

}

