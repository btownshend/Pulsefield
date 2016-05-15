import processing.core.*;

import java.util.Random;

import processing.core.PGraphics;
// A simple Particle class

class Particle {
	static Random rng = new Random();
	PVector location;
	PVector velocity;
	PVector acceleration;
	float lifespan, maxlifespan;
	int color;
	PImage img;


	Particle(PVector l, PVector v, int color, PImage img) {
		this.img=img;
		//float kscale=0.5f;
		//this.color=((int)((color&0xff)*kscale)) | ((int)(((color>>8)&0xff)*kscale)<<8) | ((int)(((color>>16)&0xff)*kscale)<<16) | 0xff000000;
		//PApplet.println("color = "+Integer.toHexString(color)+" -> "+Integer.toHexString(this.color));
		this.color=color;
		acceleration = new PVector(0f, -0.03f/300);
		velocity = new PVector((float)rng.nextGaussian()*0.3f/300+v.x, (float)rng.nextGaussian()*0.3f/300 + v.y);
		location = l.copy();
		maxlifespan = 500.0f;
		lifespan = maxlifespan;
	}

	void attractor(PVector c, float force) {
		// Attraction to c
		PVector dir=PVector.sub(c, location);
		float dist2=dir.magSq();
		dir.normalize();
		dir.mult(100f/300*force/PApplet.max(100.0f/300, dist2));
		dir.rotate(90);  
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
	void draw(PGraphics parent, PVector wsize) {
		//println("display(): location="+location);
		parent.imageMode(PConstants.CENTER);
		//float kscale= 0.05f;//lifespan/maxlifespan/10;
		float kscale=(float)Math.pow(1.0-lifespan/(2*maxlifespan),0.1)/100+0.05f;
		kscale=0.05f;
		parent.tint(color,(int)(kscale*255.0));
		parent.image(img, (location.x+1)*wsize.x/2, (location.y+1)*wsize.y/2);

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

