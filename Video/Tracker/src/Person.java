import processing.core.PApplet;
import processing.core.PVector;

public class Person {
	float lastmovetime;   // Last moved time in seconds
	static float averagingTime =0.1f;   // Averaging time in seconds
	private PVector position;  // Position in meters (in absolute coordinate)
	private PVector velocity;  // Velocity in meters/sec
	int channel;
	int id;
	int groupid;
	int groupsize;
	Leg[] legs;
	
	public Person(PVector origin, int channel, int id) {
		this.lastmovetime= 0f;
		this.position=new PVector(origin.x,origin.y);
		this.velocity=new PVector(0f,0f);
		this.channel = channel;
		this.id = id;
		this.groupid = id;
		this.groupsize = 1;
		this.legs=new Leg[2];
		for (int i=0;i<legs.length;i++)
			this.legs[i]=new Leg();
	}
	
	// Convert to normalized position - in range [-1,1] for extent of pulsefield
	PVector getNormalizedPosition() {
		return Tracker.mapPosition(position);
	}

	void setNormalizedPosition(PVector position) {
		this.position = Tracker.unMapPosition(position);
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
		position=newpos;
		velocity=newvel;
		this.groupid=groupid;
		this.groupsize=groupsize;
	}
	
	PVector getOriginInMeters() {
		return position;
	}
		
	PVector getVelocityInMeters() {
		return velocity;
	}
	
	void setVelocity(PVector vel) { velocity=vel; }
	
	float getLegSeparationInMeters() {
		return 0.1f;  // TODO
	}
	Boolean isMoving() {
		return velocity.mag() > 0.1;
	}
}
