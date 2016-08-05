import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PShape;
import processing.core.PVector;

public abstract class Visualizer {
	String name;
	boolean selectable;
	
	Visualizer() {
		name="??";
	}

	static public void drawText(PGraphics g, float height, String text, float x, float y) {
		// Draw text
		// Need to flip x-axis to get a reasonable size
		// Also, fonts are built based on the size, interpreted as pixels
		// So, setup so the textSizes are in cm rather than m (so a 10cm font will use a 10pt rendering)
		g.pushMatrix();
		g.translate(x, y);
		g.scale(-0.01f,0.01f);
		g.textSize(height*100);
		g.text(text, 0, 0);
		g.popMatrix();
	}
	
	static public void drawText(PGraphics g, float height, String text, float x, float y, float x2, float y2) {
		// Draw text
		// Need to flip x-axis to get a reasonable size
		// Also, fonts are built based on the size, interpreted as pixels
		// So, setup so the textSizes are in cm rather than m (so a 10cm font will use a 10pt rendering)
		g.pushMatrix();
		g.translate(x, y);
		float scale=100f;
		g.scale(-1/scale,1/scale);
		g.textSize(height*scale);
		g.text(text, 0, 0,x2*scale,y2*scale);
		g.popMatrix();
	}
	
	public static void drawShape(PGraphics g, PShape shape, float x, float y, float width, float height) {
		// TODO Auto-generated method stub
		g.pushMatrix();
		g.translate(x, y);
		g.scale(-1,1);
		g.shape(shape,0,0,width, height);
		g.popMatrix();
	}
	
	public void drawWelcome(Tracker t, PGraphics g) {
		final PVector center=Tracker.getFloorCenter();
		final float textHeight=0.2f;   // Height in meters
		final float lineSize=textHeight*2;
		g.fill(50, 255, 255);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.stroke(255);
		g.strokeWeight(0.02f);
		drawText(g,textHeight,"Welcome to the", center.x,center.y-lineSize*2);
		drawText(g,textHeight*1.33f,"PULSEFIELD", center.x, center.y-lineSize);
		drawText(g,textHeight,name, center.x,center.y+lineSize);
		drawText(g,textHeight,"Please enter...", center.x,center.y+2.5f*lineSize);
	}
	
	// Clean up graphics context to "default" state
	public void initializeContext(Tracker t, PGraphics g) { 
		g.colorMode(PConstants.RGB, 255);
		g.rectMode(PApplet.CORNER);
		g.smooth();
		g.stroke(255);
		g.imageMode(PConstants.CORNER);
		g.noTint();
		g.strokeWeight(0.02f);
		// With the scaling from meters to pixels, processing by default will generate a font 
		// with point-size using the textSize argument UNSCALED.  This is then scaled by the transform
		// So, if there is 10x scaling and you use textSize(1.0), the font will be generated a 1pixel size
		// then scaled up.
		// Hack this by using a font that is larger than needed, which then will get scaled down 
		//g.textFont(t.createFont("Arial",50f));  // This is a memory leak!
	}
	
	public void draw(Tracker t, PGraphics g, People p) {
		initializeContext(t,g);
		g.background(0, 0, 0); 
		if (p.pmap.isEmpty())
			drawWelcome(t, g);
		if (t.drawBorders)
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
	public void setSelectable(boolean s) { selectable=s; }
	
	public void drawBorders(PGraphics g) {
		this.drawBorders(g, 0.02f,0xff00ff00,255);
	}
	
	public void drawBorders(PGraphics g, float strokeWeight, int color,int alpha) {
		g.pushStyle();
		g.stroke(color,alpha);
		g.fill(color);
		g.strokeWeight(strokeWeight);

		g.line(Tracker.minx+strokeWeight/2, Tracker.miny+strokeWeight/2, Tracker.minx+strokeWeight/2, Tracker.maxy-strokeWeight/2);
		g.line(Tracker.minx+strokeWeight/2, Tracker.miny+strokeWeight/2, Tracker.maxx-strokeWeight/2, Tracker.miny+strokeWeight/2);
		g.line(Tracker.maxx-strokeWeight/2, Tracker.maxy-strokeWeight/2, Tracker.minx+strokeWeight/2, Tracker.maxy-strokeWeight/2);
		g.line(Tracker.maxx-strokeWeight/2, Tracker.maxy-strokeWeight/2, Tracker.maxx-strokeWeight/2, Tracker.miny+strokeWeight/2);
		g.ellipseMode(PConstants.CENTER);
		g.ellipse(0f,0f,0.1f,0.1f);
		g.popStyle();
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
