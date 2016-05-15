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

	public void draw(PGraphics parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);

		parent.ellipseMode(PConstants.CENTER);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			for (int i=0;i<ps.legs.length;i++) {
				Leg leg=ps.legs[i];
				PVector sz=Tracker.mapVelocity(new PVector(leg.getDiameterInMeters(),leg.getDiameterInMeters()));
				PVector pos=Tracker.floorToNormalized(leg.getOriginInMeters());
				parent.ellipse((pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2, sz.x*wsize.x/2, sz.y*wsize.y/2);
			}
		}
	}
}

