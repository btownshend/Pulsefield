import processing.core.PVector;
import processing.core.PApplet;

public class Position {
	PVector origin;
	PVector avgspeed; // Average speed in pixels/second
	float lastmovetime;   // Last moved time in seconds
	static float averagingTime =1.0f;   // Averaging time in seconds
	int channel;
	boolean enabled;

	public Position(PVector origin, int channel) {
		this.origin=origin;
		this.avgspeed=new PVector(0f,0f);
		this.lastmovetime= 0f;
		this.channel = channel;
	}
	
	public Position(int channel) {
		this.origin = new PVector(0f,0f);
		this.avgspeed=new PVector(0f,0f);
		this.lastmovetime = 0f;
		this.channel = channel;
	}

	void move(PVector newpos, float elapsed) {
		//PApplet.println("move("+newpos+","+elapsed+"), lastmovetime="+lastmovetime);
		if (lastmovetime!=0.0 && elapsed>lastmovetime) {
			PVector moved=newpos.get();
			moved.sub(origin);
			moved.mult(1.0f/(elapsed-lastmovetime));
			// Running average using exponential decay
			float k=(elapsed-lastmovetime)/averagingTime;
			if (k>1.0f)
				k=1.0f;
			avgspeed.mult(1-k);
			moved.mult(k);
			avgspeed.add(moved);
			//PApplet.println("\t\t\t\tk="+k+", Speed="+avgspeed);
		}
		origin=newpos;
		lastmovetime=elapsed;
	}
	
	void enable(boolean en) {
		if (en!=enabled) {
			enabled=en;
			lastmovetime=0;
		}
	}

}
