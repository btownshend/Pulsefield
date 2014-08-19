import processing.core.PApplet;
import processing.core.PVector;

public class Person {
	private PVector position;  // Position in meters (in absolute coordinate)
	private PVector velocity;  // Velocity in meters/sec
	int channel;
	int id;
	int groupid;
	int groupsize;
	float sep;   		// Leg separation in meters
	float diam;			// Leg diameter in meters
	float userData;		// extra data for apps
	Leg[] legs;
	
	public Person(PVector origin, int channel, int id) {
		this.position=new PVector(origin.x,origin.y);
		this.velocity=new PVector(0f,0f);
		this.channel = channel;
		this.id = id;
		this.groupid = id;
		this.groupsize = 1;
		this.legs=new Leg[2];
		for (int i=0;i<legs.length;i++)
			this.legs[i]=new Leg(origin);
		this.userData=0;
	}
	
	// Convert to normalized position - in range [-1,1] for extent of pulsefield
	PVector getNormalizedPosition() {
		return Tracker.floorToNormalized(position);
	}

	PVector getNormalizedPosition(boolean preserveAspect) {
		return Tracker.floorToNormalized(position, preserveAspect);
	}
	
	void setNormalizedPosition(PVector position) {
		this.position = Tracker.normalizedToFloor(position);
	}

	PVector getNormalizedVelocity() {
		return Tracker.mapVelocity(velocity);
	}

	int getcolor(PApplet parent) {
		final int colors[] = {0xffffffff, 0xff00ff00, 0xff0000ff, 0xffFFFF00, 0xffFF00FF, 0xff00ffff};
		
		int col=colors[(id-1)%colors.length];
		//PApplet.println("Color="+String.format("%x", col));
		return col;
	}

	void move(PVector newpos, PVector newvel, int groupid, int groupsize, float elapsed) {
//		PApplet.println("Set ID "+id+" to pos="+newpos+", vel="+newvel);
		position=new PVector(newpos.x,newpos.y);
		velocity=new PVector(newvel.x,newvel.y);
		this.groupid=groupid;
		this.groupsize=groupsize;
	}
	
	PVector getOriginInMeters() {
		return position;
	}
		
	PVector getVelocityInMeters() {
		return velocity;
	}
	
	void setVelocity(PVector vel) { velocity=new PVector(vel.x,vel.y); }
	
	float getLegSeparationInMeters() {
		return sep;
	}
	void setLegSeparation(float sep) {
		this.sep=sep;
	}
	void setLegDiameter(float diam) {
		this.diam=diam;
	}
	Boolean isMoving() {
		return velocity.mag() > 0.1;
	}
}
