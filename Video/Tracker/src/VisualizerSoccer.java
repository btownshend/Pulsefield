import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PShape;
import processing.core.PVector;

class Ball {
	PVector position;  // Position, velocity in absolute (meters) coordinate space (maps to entire active area)
	PVector velocity;
	final float deceleration=0.07f*9.81f;  // Decleration while rolling in m/s^2 (mu_r=0.07 from http://www.stmarys-ca.edu/sites/default/files/attachments/files/JGrider.pdf )
	final float restitution=0.7f;   // Coeff of restitution (see http://www.mathematicshed.com/uploads/1/2/5/7/12572836/physicsofkickingsoccerball.pdf )
	final float mass=0.430f;			// Mass of ball in kg (FIFA says 410-450g )
	final float radius=(float)(0.69/2/Math.PI);			// Radius of ball in meters (FIFA say 68-70cm in circumference )
	PShape ballShape;
	
	public Ball(PVector position, PVector velocity) {
		this.position=position;
		this.velocity=velocity;
		ballShape=null;
	}
	
	public void draw(PApplet parent, PVector wsize) {
		if (ballShape==null)
			ballShape=parent.loadShape("Soccerball.svg");
		final int color=0xffffffff;
		parent.ellipseMode(PConstants.CENTER);
		parent.fill(color,0);
		parent.strokeWeight(2.0f);
		parent.stroke(color,255);
//		PApplet.println("Ball at "+((position.x+1)*wsize.x/2)+","+((position.y+1)*wsize.y/2));
		PVector p=Tracker.mapPosition(position);
		PVector nRadius=Tracker.mapVelocity(new PVector(radius,radius));
//		parent.ellipse((p.x+1)*wsize.x/2, (p.y+1)*wsize.y/2, nRadius.x*2*wsize.x/2, nRadius.y*2*wsize.y/2);
		parent.shape(ballShape,(p.x+1-nRadius.x)*wsize.x/2, (p.y+1-nRadius.y)*wsize.y/2,nRadius.x*2*wsize.x/2, nRadius.y*2*wsize.y/2);
	}
	
	public void drawLaser(Laser laser,PApplet parent) {
//		laser.circle(position.x,position.y, radius);
		laser.svgfile("Soccerball.svg", position.x, position.y, radius*2, 0f);
	}
	
	public void update(PApplet parent) {
		float elapsed=1.0f/parent.frameRate;
		position.add(PVector.mult(velocity,elapsed));
		if (position.x+radius>Tracker.rawmaxx && velocity.x>0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.minx+","+Tracker.rawminy+","+Tracker.rawmaxx+","+Tracker.rawmaxy);
			velocity.x*=-restitution;
			position.x=2*(Tracker.rawmaxx-radius)-position.x;
		}
		if (position.x-radius<Tracker.rawminx && velocity.x<0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.rawminx+","+Tracker.rawminy+","+Tracker.rawmaxx+","+Tracker.rawmaxy);
			velocity.x*=-restitution;
			position.x=2*(Tracker.rawminx+radius)-position.x;
		}
		if (position.y+radius>Tracker.rawmaxy && velocity.y>0 ) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.rawminx+","+Tracker.rawminy+","+Tracker.rawmaxx+","+Tracker.rawmaxy);
			PApplet.println("Position.y="+position.y+", radius="+radius+", maxy="+Tracker.rawmaxy);
			velocity.y*=-restitution;
			position.y=2*(Tracker.rawmaxy-radius)-position.y;
		}
		if (position.y-radius<Tracker.rawminy && velocity.y<0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.rawminx+","+Tracker.rawminy+","+Tracker.rawmaxx+","+Tracker.rawmaxy);
			velocity.y*=-restitution;
			position.y=2*(Tracker.rawminy+radius)-position.y;
		}
		velocity.mult(1-deceleration*elapsed);
//		PApplet.println("New ball position="+position+", velocity="+velocity+", inCollision="+inCollision);
	}
	
	// Check for collision with person at position p
	public void collisionDetect(Person p) {
		for (int i=0;i<2;i++) {
			Leg leg=p.legs[i];

			float sep=PVector.dist(leg.getOriginInMeters(),position);
			float minSep=leg.getDiameterInMeters()/2+radius;
			if (sep<minSep) { // && PVector.dot(velocity, leg.getVelocityInMeters())<=0) {
				PApplet.println("Ball at "+position+" with velocity="+velocity+" collided with leg at "+leg.getOriginInMeters()+", velocity="+leg.getVelocityInMeters()+", minsep="+minSep);
				// Perform a kick in the ball-not-moving frame of reference
				// Calculate a kick velocity
				PVector kickvelocity=PVector.mult(PVector.sub(leg.getVelocityInMeters(),velocity),leg.getMassInKg()/(leg.getMassInKg()+mass)*(1+restitution));
				// And add the kickvelocity to its current velocity (ie switch back to work frame of reference)
				velocity.add(kickvelocity);
				PApplet.println("New ball velocity="+velocity);
				// And make sure the ball stays outside the leg
				PVector shiftDir=new PVector(velocity.x,velocity.y);
				shiftDir.setMag(minSep-sep);
				position.add(shiftDir);
			}
		}
	}
}

public class VisualizerSoccer extends VisualizerDot {
	Ball ball;
	
	VisualizerSoccer(PApplet parent) {
		super(parent);
	}

	@Override
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		// Other initialization when this app becomes active
		ball=new Ball(Tracker.unMapPosition(new PVector(0f,0f)),Tracker.unMapPosition(new PVector(0.1f,0.2f)));
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
		ball=null;
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
		ball.update(parent);
		for (Person ps: p.pmap.values()) {  
			ball.collisionDetect(ps);
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		ball.draw(parent,wsize);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		Laser laser=Laser.getInstance();
		laser.bgBegin();   // Start a background drawing
		ball.drawLaser(laser,parent);
		laser.bgEnd();
	}
}
