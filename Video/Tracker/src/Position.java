import processing.core.PApplet;
import processing.core.PVector;

public class Position {
	PVector origin;
	PVector avgspeed; // Average speed in pixels/second
	float lastmovetime;   // Last moved time in seconds
	static float averagingTime =1.0f;   // Averaging time in seconds
	int channel;
	int id;
	int groupid;
	int groupsize;

	public Position(PVector origin, int channel, int id) {
		this.origin=origin;
		this.avgspeed=new PVector(0f,0f);
		this.lastmovetime= 0f;
		this.channel = channel;
		this.id = id;
		this.groupid = id;
		this.groupsize = 1;
	}
	
	public Position(int channel) {
		this.origin = new PVector(0f,0f);
		this.avgspeed=new PVector(0f,0f);
		this.lastmovetime = 0f;
		this.channel = channel;
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
		this.groupid=groupid;
		this.groupsize=groupsize;
	}
	

}
