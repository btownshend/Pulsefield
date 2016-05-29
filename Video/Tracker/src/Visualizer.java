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

	public void drawWelcome(Tracker t, PGraphics g) {
		final PVector center=Tracker.getFloorCenter();
		final float textHeight=0.2f;   // Height in meters
		final float lineSize=textHeight*2;
		g.fill(50, 255, 255);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.textSize(textHeight);
		g.stroke(255);
		g.text("Welcome to the", center.x,center.y-lineSize*2);
		g.textSize(textHeight*1.33f);
		g.text("PULSEFIELD", center.x, center.y-lineSize);
		g.textSize(textHeight);
		g.text(name, center.x,center.y+lineSize);
		g.text("Please enter...", center.x,center.y+2.5f*lineSize);
	}
	
	// Clean up graphics context to "default" state
	public void initializeContext(Tracker t, PGraphics g) { 
		g.colorMode(PConstants.RGB, 255);
		g.rectMode(PApplet.CORNER);
		g.smooth();
		g.stroke(255);
		g.imageMode(PConstants.CORNER);
		g.noTint();
		// With the scaling from meters to pixels, processing by default will generate a font 
		// with point-size using the textSize argument UNSCALED.  This is then scaled by the transform
		// So, if there is 10x scaling and you use textSize(1.0), the font will be generated a 1pixel size
		// then scaled up.
		// Hack this by using a font that is larger than needed, which then will get scaled down 
		g.textFont(t.createFont("Arial",30f),30f);
	}
	
	public void draw(Tracker t, PGraphics g, People p) {
		initializeContext(t,g);
		g.background(0, 0, 0); 
		if (p.pmap.isEmpty())
			drawWelcome(t, g);
		drawBorders(g);
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
	
	public void drawBorders(PGraphics g) {
		this.drawBorders(g, 0.02f,0xff00ff00,255);
	}
	
	public void drawBorders(PGraphics g, float strokeWeight, int color,int alpha) {
		g.stroke(color,alpha);
		g.fill(color);
		g.strokeWeight(strokeWeight);

		g.line(Tracker.rawminx+strokeWeight/2, Tracker.rawminy+strokeWeight/2, Tracker.rawminx+strokeWeight/2, Tracker.rawmaxy-strokeWeight/2);
		g.line(Tracker.rawminx+strokeWeight/2, Tracker.rawminy+strokeWeight/2, Tracker.rawmaxx-strokeWeight/2, Tracker.rawminy+strokeWeight/2);
		g.line(Tracker.rawmaxx-strokeWeight/2, Tracker.rawmaxy-strokeWeight/2, Tracker.rawminx+strokeWeight/2, Tracker.rawmaxy-strokeWeight/2);
		g.line(Tracker.rawmaxx-strokeWeight/2, Tracker.rawmaxy-strokeWeight/2, Tracker.rawmaxx-strokeWeight/2, Tracker.rawminy+strokeWeight/2);
		g.ellipseMode(PConstants.CENTER);
		g.ellipse(0f,0f,0.1f,0.1f);
	}

	public void handleMessage(OscMessage theOscMessage) {
		PApplet.println("Unhandled OSC Message: "+theOscMessage.toString());
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
