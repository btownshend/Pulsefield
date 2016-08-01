import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PShape;
import processing.core.PVector;

class Ball {
	PVector position;  // Position, velocity in absolute (meters) coordinate space (maps to entire active area)
	PVector velocity;
	final float deceleration=0.02f*9.81f;  // Decleration while rolling in m/s^2 (mu_r=0.07 from http://www.stmarys-ca.edu/sites/default/files/attachments/files/JGrider.pdf )
	final float midaccel=0.05f*9.81f;       // Acceleration towards middle in m/s^2
	final float restitution=0.7f;   // Coeff of restitution (see http://www.mathematicshed.com/uploads/1/2/5/7/12572836/physicsofkickingsoccerball.pdf )
	final float mass=0.430f;			// Mass of ball in kg (FIFA says 410-450g )
	final float radius=(float)(0.69/2/Math.PI);			// Radius of ball in meters (FIFA say 68-70cm in circumference )
	PShape ballShape;
	
	public Ball(PVector position, PVector velocity) {
		this.position=position;
		this.velocity=velocity;
		ballShape=null;
	}
	
	public void draw(PGraphics g) {
		if (ballShape==null)
			ballShape=g.loadShape(Tracker.SVGDIRECTORY+"Soccerball.svg");

		//PApplet.println("Ball at "+position.x+","+position.y);
		g.shapeMode(PConstants.CENTER);
		Visualizer.drawShape(g, ballShape,position.x, position.y,radius*2,radius*2);
	}
	
	public void drawLaser(Laser laser,PApplet parent) {
//		laser.circle(position.x,position.y, radius);
		laser.svgfile("Soccerball-outline.svg", position.x, position.y, radius*2, 0f);
	}
	
	public void update(PApplet parent) {
		float elapsed=1.0f/parent.frameRate;
		position.add(PVector.mult(velocity,elapsed));
		if (position.x+radius>Tracker.maxx && velocity.x>0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.minx+","+Tracker.miny+","+Tracker.maxx+","+Tracker.maxy);
			velocity.x*=-restitution;
			position.x=2*(Tracker.maxx-radius)-position.x;
			impactSound(0);
		}
		if (position.x-radius<Tracker.minx && velocity.x<0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.minx+","+Tracker.miny+","+Tracker.maxx+","+Tracker.maxy);
			velocity.x*=-restitution;
			position.x=2*(Tracker.minx+radius)-position.x;
			impactSound(0);
		}
		if (position.y+radius>Tracker.maxy && velocity.y>0 ) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.minx+","+Tracker.miny+","+Tracker.maxx+","+Tracker.maxy);
			PApplet.println("Position.y="+position.y+", radius="+radius+", maxy="+Tracker.maxy);
			velocity.y*=-restitution;
			position.y=2*(Tracker.maxy-radius)-position.y;
			impactSound(0);
		}
		if (position.y-radius<Tracker.miny && velocity.y<0) {
			PApplet.println("Bounce off wall: position="+position+", bounds="+Tracker.minx+","+Tracker.miny+","+Tracker.maxx+","+Tracker.maxy);
			velocity.y*=-restitution;
			position.y=2*(Tracker.miny+radius)-position.y;
			impactSound(0);
		}
		velocity.mult(1-deceleration*elapsed);
		PVector toMiddle=PVector.sub(new PVector((Tracker.maxx+Tracker.minx)/2,(Tracker.miny+Tracker.maxy)/2),position);
		if (toMiddle.mag() > 0)
			velocity.add(PVector.mult(toMiddle, midaccel*elapsed/position.mag()));
//		PApplet.println("New ball position="+position+", velocity="+velocity+", inCollision="+inCollision);
	}
	
	public void impactSound(int k) {
		TrackSet ts=Ableton.getInstance().trackSet;
		int track=ts.firstTrack;
		int nclips=Ableton.getInstance().getTrack(track).numClips();
		PApplet.println("Track="+track+", nclips="+nclips);
		if (nclips!=-1)
			Ableton.getInstance().playClip(track,k%nclips);
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
				impactSound(1);
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
		Laser.getInstance().setFlag("legs",1.0f);
		// Other initialization when this app becomes active
		Ableton.getInstance().setTrackSet("Soccer");
		ball=null;
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
		ball=null;
	}

	@Override
	public void update(PApplet parent, People p) {
		if (ball==null)
			ball=new Ball(new PVector((Tracker.maxx+Tracker.minx)/2,(Tracker.maxy+Tracker.miny)/2),new PVector(0f,0f));
		// Update internal state
		ball.update(parent);
		for (Person ps: p.pmap.values()) {  
			ball.collisionDetect(ps);
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		if (ball!=null)
			ball.draw(g);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		Laser laser=Laser.getInstance();
		laser.bgBegin();   // Start a background drawing
		if (ball!=null)
			ball.drawLaser(laser,parent);
		laser.bgEnd();
	}
}
