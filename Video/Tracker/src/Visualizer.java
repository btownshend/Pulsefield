import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

public abstract class Visualizer {
	String name;
	
	Visualizer() {
		name="??";
	}

	public void drawWelcome(PApplet parent, PVector wsize) {
		parent.fill(50, 255, 255);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.textSize(45);
		parent.stroke(255);
		final float lineSize=wsize.y/8;
		parent.text("Welcome to the", wsize.x/2,wsize.y/2-lineSize);
		parent.textSize(60);
		parent.text("PULSEFIELD", wsize.x/2,wsize.y/2);
		parent.textSize(45);
		parent.text(name, wsize.x/2,wsize.y/2+lineSize);
		parent.text("Please enter...", wsize.x/2,wsize.y/2+2.5f*lineSize);
	}
	
	public void draw(PApplet parent, People p, PVector wsize) {
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		if (p.pmap.isEmpty())
			drawWelcome(parent,wsize);
		drawBorders(parent, false, wsize);
	}

	// Draw to laser
	public void drawLaser(PApplet parent, People p) {
	;	
	}
	
	abstract public void update(PApplet parent, People p);
	public void start() {;}
	public void stop() {
		Ableton.getInstance().setTrackSet(null);
		Ableton.getInstance().stop();
		Laser.getInstance().reset();
	}

	public void stats() { }

	public void setName(String name) { this.name=name; }
	
	public void drawBorders(PApplet parent, boolean octagon, PVector wsize) {
		this.drawBorders(parent, octagon, wsize, 2.0f,127,255);
	}
	
	public void drawBorders(PApplet parent, boolean octagon, PVector wsize, float strokeWeight, int color,int alpha) {
		octagon=false;
		parent.stroke(color,alpha);
		parent.fill(0);
		parent.strokeWeight(strokeWeight);
		if (octagon) {
			parent.beginShape();
			float gapAngle=(float)(10f*Math.PI /180);
			for (float angle=gapAngle/2;angle<2*Math.PI;angle+=(2*Math.PI-gapAngle)/8)
				parent.vertex((float)((Math.sin(angle+Math.PI)+1)*wsize.x/2),(float)((Math.cos(angle+Math.PI)+1)*wsize.y/2));
			parent.endShape(PConstants.OPEN);
		} else {
			parent.line(0, 0, wsize.x-1, 0);
			parent.line(0, 0, 0, wsize.y-1);
			parent.line(wsize.x-1, 0, wsize.x-1, wsize.y-1);
			parent.line(0, wsize.y-1, wsize.x-1, wsize.y-1);
		}
		// Narrow remaining window
		parent.translate(wsize.x/2, wsize.y/2);
		PVector scale=new PVector((wsize.x-strokeWeight*2)/wsize.x,(wsize.y-strokeWeight*2)/wsize.y);
//		PApplet.println("Scale="+scale);
		parent.scale(scale.x,scale.y);
		parent.translate(-wsize.x/2, -wsize.y/2);
	}

	public void handleMessage(OscMessage theOscMessage) {
		PApplet.println("Unhandled OSC Message: "+theOscMessage.toString());
	}

	public PVector convertToScreen(PVector p, PVector wsize) {
		return new PVector((p.x+1)*wsize.x/2,(p.y+1)*wsize.y/2);
	}
}
