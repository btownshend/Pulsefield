// A simple Particle class

class Particle {
  PVector location;
  PVector velocity;
  PVector acceleration;
  float lifespan, maxlifespan;
  color col;
  int id;
  PImage img;


  Particle(PVector l, PVector v, color col, PImage img, int id) {
    this.img=img;
    this.col=col;
    this.id=id;
    acceleration = new PVector(0, -0.03);
    velocity = new PVector(randomGaussian()*0.3+v.x, randomGaussian()*0.3 + v.y);
    location = l.get();
    maxlifespan = 500.0;
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
    dir.mult(100*force/max(100, dist2));
    dir.rotate(90);  
    velocity.add(dir);
  }

  void push(PVector c, PVector pushvel) {
    // Push particles with pushvel velocity
    // Scales with distance
    float dist=PVector.sub(c, location).mag();
    PVector dir=pushvel.get(); 
    float releffect = max(0.0,1.0-dist/100);
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
    imageMode(CENTER);
    tint(col, lifespan/maxlifespan*255.0/20);
    image(img, location.x, location.y);

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

