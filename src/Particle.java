import processing.core.*;


// A simple Particle class

class Particle {
	PApplet parent;
	PVector location;
	PVector velocity;
	PVector acceleration;
	float lifespan, maxlifespan;
	int col;
	int id;
	PImage img;


	Particle(PApplet parent, PVector l, PVector v, int col, PImage img, int id) {
		this.parent=parent;
		this.img=img;
		this.col=col;
		this.id=id;
		acceleration = new PVector(0f, -0.03f);
		velocity = new PVector(parent.randomGaussian()*0.3f+v.x, parent.randomGaussian()*0.3f + v.y);
		location = l.get();
		maxlifespan = 500.0f;
		lifespan = maxlifespan;
	}

	void run() {
		display();
		update();
	}

	void attractor(PVector c, float force) {
		// Attraction to c
		PVector dir=PVector.sub(c, location);
		float dist2=dir.magSq();
		dir.normalize();
		dir.mult(100*force/PApplet.max(100.0f, dist2));
		dir.rotate(90);  
		velocity.add(dir);
	}

	void push(PVector c, PVector pushvel) {
		// Push particles with pushvel velocity
		// Scales with distance
		float dist=PVector.sub(c, location).mag();
		PVector dir=pushvel.get(); 
		float releffect = PApplet.max(0.0f,1.0f-dist/100f);
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
	void display() {
		//println("display(): location="+location);
		parent.imageMode(PConstants.CENTER);
		parent.tint(col, (float)(lifespan/maxlifespan*255.0/20));
		parent.image(img, location.x, location.y);

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

