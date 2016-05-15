import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public abstract class Visualizer {
	String name;
	
	Visualizer() {
		name="??";
	}

	public void drawWelcome(PGraphics g, PVector wsize) {
		g.fill(50, 255, 255);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.textSize(45);
		g.stroke(255);
		final float lineSize=wsize.y/8;
		g.text("Welcome to the", wsize.x/2,wsize.y/2-lineSize);
		g.textSize(60);
		g.text("PULSEFIELD", wsize.x/2,wsize.y/2);
		g.textSize(45);
		g.text(name, wsize.x/2,wsize.y/2+lineSize);
		g.text("Please enter...", wsize.x/2,wsize.y/2+2.5f*lineSize);
	}
	
	// Clean up graphics context to "default" state
	public void initializeContext(PGraphics g) { 
		g.colorMode(PConstants.RGB, 255);
		g.rectMode(PApplet.CORNER);
		g.smooth();
		g.stroke(255);
		g.imageMode(PConstants.CORNER);
		g.noTint();
	}
	
	public void draw(Tracker t, PGraphics g, People p, PVector wsize) {
		initializeContext(g);
		g.background(0, 0, 0); 
		if (p.pmap.isEmpty())
			drawWelcome(g,wsize);
		drawBorders(g, false, wsize);
	}

	// Draw to laser
	public void drawLaser(PApplet parent, People p) {
	;	
	}
	
	abstract public void update(PApplet parent, People p);
	
	// NOTE: start and stop don't have PApplet args since that would make it easy to issue drawing commands, which is bad because
	// start/stop may be called from threads other than the draw() thread and OpenGL doesn't like that (causes segmentation fault)
	public void start() {;}
	public void stop() {
		Ableton.getInstance().setTrackSet(null);
		Ableton.getInstance().stop();
		Laser.getInstance().reset();
	}

	public void stats() { }

	public void setName(String name) { this.name=name; }
	
	public void drawBorders(PGraphics parent, boolean octagon, PVector wsize) {
		this.drawBorders(parent, octagon, wsize, 2.0f,127,255);
	}
	
	public void drawBorders(PGraphics parent, boolean octagon, PVector wsize, float strokeWeight, int color,int alpha) {
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
	
	// Create a bezier path from p1 to p2 that vibrates over time
	public PVector[] vibratingPath(PVector p1, PVector p2, int mode, float freq, float amplitude, float time) {
		int ncurves=mode*2;
		int npoints=3*ncurves+1;
		PVector result[]=new PVector[npoints];
		result[0]=new PVector(0f,0f);
		int last=1;
		for (int i=0;i<mode;i++) {
			float amp=(float) (((i%2==0)?1:-1)*amplitude*Math.sin(2*Math.PI*freq*time));
			float offset=(float) (Math.PI*i);
			result[last++]=new PVector(offset+0.5251f,.5251f*amp);
			result[last++]=new PVector(offset+1.005f,1f*amp);
			result[last++]=new PVector((float) (offset+Math.PI/2),1f*amp);
			result[last++]=new PVector((float) (offset+Math.PI-1.005f),1f*amp);
			result[last++]=new PVector((float) (offset+Math.PI-.5251f),.5251f*amp);
			result[last++]=new PVector((float) (offset+Math.PI),0f*amp);
		}
		// Transform points to fall on p1-p2 line
//		PApplet.println("vibratingPath("+p1+","+p2+","+mode+","+freq+","+amplitude+","+time+"):");
		PVector dir=PVector.sub(p2,p1); dir=PVector.div(dir, dir.mag());
		for (int i=0;i<npoints;i++) {
//			PApplet.print(result[i].x+","+result[i].y+",   ");
			float d=(float) (result[i].x/(Math.PI*mode))*PVector.dist(p1,p2);
			float y=result[i].y;
			result[i].x=d*dir.x+y*dir.y+p1.x;
			result[i].y=d*dir.y+y*dir.x+p1.y;
//			PApplet.println(result[i].x+","+result[i].y);
		}
		return result;
	}
}
