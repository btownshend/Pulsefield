import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
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

class Goal {
	PVector pos, dir; // Dir is direction from front of goal to back
	float width, depth;
	PImage img;
	PShape outline;
	boolean scoreDelay=false;  // True immediately after a goal
	
	Goal() {
		this.img=Tracker.theTracker.loadImage("soccer/goal.jpg");
		// Vector pointing into goal
	}
	
	public void setpos(PVector pos, PVector dir, float width, float depth) {
		this.pos=pos;
		this.dir=dir;
		this.width=width;
		this.depth=depth;
	}
	
	void draw(PGraphics g) {
		PVector aim=dir;
		
		g.pushMatrix();
		g.translate(pos.x, pos.y);
		g.rotate((float)Math.atan2(aim.y,aim.x));
		g.tint(255);
		g.imageMode(PConstants.CENTER);
		g.image(img,0f,0f,depth,width);
		//PApplet.println("Drawing image at "+pos+"; width="+width+", depth="+depth);
		g.popMatrix();
		
		if (scoreDelay)
			g.stroke(255,0,0);
		else
			g.stroke(0,255,0);
		g.noFill();
		
		outline=g.createShape();
		outline.noFill();
		outline.beginShape();
		outline.vertex(pos.x-depth/2*aim.x+width/2*aim.y, pos.y-depth/2*aim.y-width/2*aim.x);
		outline.vertex(pos.x+depth/2*aim.x+width/2*aim.y, pos.y+depth/2*aim.y-width/2*aim.x);
		outline.vertex(pos.x+depth/2*aim.x-width/2*aim.y, pos.y+depth/2*aim.y+width/2*aim.x);
		outline.vertex(pos.x-depth/2*aim.x-width/2*aim.y, pos.y-depth/2*aim.y+width/2*aim.x);
		outline.endShape();
		g.shape(outline);
	}
	
	float pt2line(PVector p1, PVector p2, PVector pt) {
		PVector v1=PVector.sub(pt, p1);
		PVector delta=PVector.sub(p2, p1);
		float len=PVector.dot(v1, delta)/delta.magSq();
		if (len<0)
			return PVector.sub(pt, p1).mag();
		else if (len>1)
			return PVector.sub(pt,p2).mag();
		else {
			PVector p=PVector.add(p1, PVector.mult(delta, len)); // Closest pt
			return PVector.sub(p, pt).mag();
		}
	}
	
	// Check if ball hits goal and return new velocity if so
	// return true if this counts as a goal
	boolean collisionDetect(Ball b) {
		if (outline==null) {
			PApplet.println("no outline");
			return false;
		}
			
		PVector p1=outline.getVertex(0);
		float mindist=1000;
		for (int i=0;i<outline.getVertexCount()-1;i++) {
			PVector p2=outline.getVertex((i+1)%outline.getVertexCount());
			float dist=pt2line(p1,p2,b.position);
			if (dist<mindist)
				mindist=dist;
			if (dist<b.radius) {
				float dist2=pt2line(p1,p2,PVector.add(b.position, PVector.mult(b.velocity,1e-5f)));
				// Check if its getting closer
				if (dist2<dist) {
					boolean frontSide=PVector.dot(dir, b.velocity)>0;
					PApplet.println("ball at "+b.position+" hit goal line "+i+" between "+p1+" and "+p2);
					PApplet.println("dist="+dist+", dist2="+dist+", frontside="+frontSide);
					// Resolve velocity into parallel and perp components
					PVector parallel=PVector.sub(p2,p1);
					parallel.normalize();
					parallel.mult(b.velocity.dot(parallel));
					PVector perp=PVector.sub(b.velocity, parallel);
					PApplet.println("vel="+b.velocity+"="+parallel+"(para) + "+perp);
					b.velocity = PVector.sub(parallel,perp);  // Bounce
					b.impactSound(2);
					if (i==1 && !scoreDelay && frontSide) {
						// Through the goal line
						PApplet.println("Goal! dot(aim,vel) = ",PVector.dot(dir, b.velocity));
						scoreDelay=true;
						b.impactSound(3);
						return true;
					}
				}
			}
			p1=p2;
		}
		// Check if ball has cleared goal area
		if (mindist>1)
			scoreDelay=false;   // Can now score again
		return false;
	}
}

public class VisualizerSoccer extends VisualizerDot {
	Ball ball;
	Goal goals[];
	int score[];
	boolean goalsAtEnds = true;
	
	VisualizerSoccer(PApplet parent) {
		super(parent);
		goals=new Goal[2];
		for (int i=0;i<goals.length;i++)
			goals[i]=new Goal();
		score=new int[2];
	}

	@Override
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",1.0f);
		// Other initialization when this app becomes active
		Ableton.getInstance().setTrackSet("Soccer");
		ball=null;
		score[0]=0;
		score[1]=0;
		Tracker.theTracker.setborders(1.0f);
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
		ball=null;
		Tracker.theTracker.setborders(0.0f);
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

		for (int i=0;i<goals.length;i++) {
			if (goals[i]!=null && goals[i].collisionDetect(ball)) {
				score[i]+=1;
			}
	
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		
		if (goalsAtEnds) {
			goals[0].setpos(new PVector(Tracker.getFloorCenter().x+1.0f,Tracker.getFloorCenter().y+Tracker.getFloorSize().y*0.5f-0.5f),
					new PVector(0f,1.0f),
					1.0f,0.25f);
			goals[1].setpos(new PVector(Tracker.getFloorCenter().x-1.0f,Tracker.getFloorCenter().y-Tracker.getFloorSize().y*0.5f+0.5f),
					new PVector(0f,-1.0f),
					1.0f,0.25f);
		} else {
			goals[0].setpos(new PVector(Tracker.getFloorCenter().x+Tracker.getFloorSize().x*0.5f-0.25f,Tracker.getFloorCenter().y),
					new PVector(1.0f,0f),
					1.0f,0.25f);
			goals[1].setpos(new PVector(Tracker.getFloorCenter().x-Tracker.getFloorSize().x*0.5f+0.25f,Tracker.getFloorCenter().y),
					new PVector(0f,1.0f),
					1.0f,0.25f);
		}
		
		for (Goal goal: goals)
			if (goal != null)
				goal.draw(g);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.fill(70);
		for (int i=0;i<score.length;i++) {
			if (goals[i]!=null)
				drawText(g,0.2f,String.format("%d",score[i]), goals[i].pos.x, goals[i].pos.y+goals[i].width*0.6f);
		}
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
