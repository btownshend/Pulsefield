import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class VisualizerTestPattern extends VisualizerDot {

	public VisualizerTestPattern(PApplet parent) {
		super(parent);
	}

	@Override
	public void start() {
		super.start();
		Tracker.theTracker.useMasks=false;
		Tracker.theTracker.drawBorders=true;
		// Other initialization when this app becomes active
	}
	
	@Override
	public void stop() {
		super.stop();
		Tracker.theTracker.useMasks=true;
		Tracker.theTracker.drawBorders=false;
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		// Draw grid
		float minxpos=(float) (Math.floor(Tracker.minx*2)/2f);
		float maxxpos=(float) (Math.ceil(Tracker.maxx*2)/2f);
		float minypos=(float) (Math.floor(Tracker.miny*2)/2f);
		float maxypos=(float) (Math.ceil(Tracker.maxy*2)/2f);
		float x=minxpos;
		while (x<=maxxpos) {
			if (Math.abs(x)<0.01f)
				g.stroke(0,0,255);
			else if (Math.abs(x-Math.round(x)) < 0.01f)
				g.stroke(255);
			else
				g.stroke(20);
			g.line(x,minypos,x,maxypos);
			x+=0.5f;
		}
		float y=minxpos;
		while (y<=maxypos) {
			if (Math.abs(y-Math.round(y)) < 0.01f)
				g.stroke(255);
			else
				g.stroke(20);
			g.line(minxpos,y,maxxpos,y);
			y+=0.5f;
		}
		// Draw radial circles
		g.stroke(127,0,127);
		g.fill(0,0);
		g.ellipseMode(PConstants.CENTER);
		float maxr=Math.max(Math.max(-Tracker.minx,Tracker.maxx),Tracker.maxy);
		for (float r=0;r<=maxr;r++) {
			g.ellipse(0f, 0f, 2*r, 2*r);
		}
		// Draw axes
		g.strokeWeight(0.1f);
		g.stroke(255,0,0);
		g.line(0, 0.05f, 2, 0.05f);
		g.stroke(0,0,255);
		g.line(0,0, 0, 2);
		
		g.textAlign(PConstants.CENTER,PConstants.BASELINE);
		g.stroke(255);
		g.fill(0,0,255);
		drawText(g,0.2f,"(2,3)", 2, 3);
		// Draw any alignment corners
		g.stroke(255,0,255);
		g.strokeWeight(0.03f);
		PVector ac[]=Tracker.alignCorners;  // Need to copy since it could be changed asynchronously
		for (int i=0;i<ac.length;i++) {
			if (ac[i]==null)
				continue;
			final float CLEN=0.5f;
			double angle=Math.atan2(ac[i].y,ac[i].x)-Math.PI/4;
			g.line(ac[i].x, ac[i].y, (float)(ac[i].x+CLEN*Math.cos(angle)), (float)(ac[i].y+CLEN*Math.sin(angle)));
			angle+=Math.PI/2;
			g.line(ac[i].x, ac[i].y, (float)(ac[i].x+CLEN*Math.cos(angle)),(float)(ac[i].y+CLEN*Math.sin(angle)));
		}
		g.stroke(0,255,0);
		g.fill(0,255,0);
		if (Tracker.theTracker.lidarbg != null)
			for (PVector pt: Tracker.theTracker.lidarbg) {
				if (pt!=null)
					g.point(pt.x, pt.y);
			}

	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
//		Laser laser=Laser.getInstance();
//		laser.bgBegin();   // Start a background drawing
//		for (Position ps: p.positions.values()) {  
//			laser.cellBegin(ps.id); // Start a cell-specific drawing
//			Laser drawing code
//			laser.cellEnd(ps.id);
//		}
//		laser.bgEnd();
	}
}
