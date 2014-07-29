import processing.core.PApplet;
import processing.core.PVector;

public class Person {
	private PVector position;
	private PVector velocity; // Average speed in pixels/second
	float lastmovetime;   // Last moved time in seconds
	static float averagingTime =0.1f;   // Averaging time in seconds
	int channel;
	int id;
	int groupid;
	int groupsize;
	Leg[] legs;
	
	public Person(PVector origin, int channel, int id) {
		this.setNormalizedPosition(origin);
		this.setNormalizedVelocity(new PVector(0f,0f));
		this.lastmovetime= 0f;
		this.channel = channel;
		this.id = id;
		this.groupid = id;
		this.groupsize = 1;
		this.legs=new Leg[2];
		for (int i=0;i<legs.length;i++)
			this.legs[i]=new Leg();
	}
	
	public Person(int channel) {
		this.setNormalizedPosition(new PVector(0f,0f));
		this.setNormalizedVelocity(new PVector(0f,0f));
		this.lastmovetime = 0f;
		this.channel = channel;
	}
	
	PVector getNormalizedPosition() {
		return position;
	}

	void setNormalizedPosition(PVector position) {
		this.position = position;
	}

	PVector getNormalizedVelocity() {
		return velocity;
	}

	void setNormalizedVelocity(PVector velocity) {
		this.velocity = velocity;
	}

	int getcolor(PApplet parent) {
		final int colors[] = {0xffffffff, 0xff00ff00, 0xff0000ff, 0xffFFFF00, 0xffFF00FF, 0xff00ffff};
		
		int col=colors[(id-1)%colors.length];
		//PApplet.println("Color="+String.format("%x", col));
		return col;
	}

	void move(PVector newpos, int groupid, int groupsize, float elapsed) {
		//PApplet.println("move("+newpos+","+elapsed+"), lastmovetime="+lastmovetime);
		if (lastmovetime!=0.0 && elapsed>lastmovetime) {
			PVector moved=newpos.get();
			moved.sub(getNormalizedPosition());
			moved.mult(1.0f/(elapsed-lastmovetime));
			// Running average using exponential decay
			float k=(elapsed-lastmovetime)/averagingTime;
			if (k>1.0f)
				k=1.0f;
			getNormalizedVelocity().mult(1-k);
			moved.mult(k);
			getNormalizedVelocity().add(moved);
			PApplet.println("\t\t\t\tk="+k+", Speed="+getNormalizedVelocity());
		}
		setNormalizedPosition(newpos);
		lastmovetime=elapsed;
		this.groupid=groupid;
		this.groupsize=groupsize;
		for (int i=0;i<legs.length;i++)
			legs[i].move(newpos,getNormalizedVelocity()); // TODO - use individual estimates
	}
	
	PVector getOriginInMeters() {
		return Tracker.unMapPosition(getNormalizedPosition());
	}
		
	PVector getVelocityInMeters() {
		return Tracker.unMapPosition(getNormalizedVelocity());
	}
	
	float getLegSeparationInMeters() {
		return 0.1f;  // TODO
	}

}
