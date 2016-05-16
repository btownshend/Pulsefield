import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

// Visualizer that just displays a dot for each person

public class VisualizerDot extends Visualizer {
	
	VisualizerDot(PApplet parent) {
		super();

	}
	
	public void update(PApplet parent, People p) {
		;
	}

	public void draw(Tracker t, PGraphics g, People p, PVector wsize) {
		super.draw(t, g, p, wsize);

		g.ellipseMode(PConstants.CENTER);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			for (int i=0;i<ps.legs.length;i++) {
				Leg leg=ps.legs[i];
				PVector sz=Tracker.mapVelocity(new PVector(leg.getDiameterInMeters(),leg.getDiameterInMeters()));
				PVector pos=Tracker.floorToNormalized(leg.getOriginInMeters());
				g.ellipse((pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2, sz.x*wsize.x/2, sz.y*wsize.y/2);
			}
		}
	}
}

