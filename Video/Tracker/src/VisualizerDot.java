import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

// Visualizer that just displays a dot for each person

public class VisualizerDot extends Visualizer {
	
	VisualizerDot(PApplet parent) {
		super();

	}
	
	public void update(PApplet parent, Positions p) {
		;
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		super.draw(parent, p, wsize);

		parent.ellipseMode(PConstants.CENTER);
		for (Position ps: p.positions.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			for (int i=0;i<ps.legs.length;i++) {
				Leg leg=ps.legs[i];
				float sz=Tracker.normalizeDistance(leg.getDiameterInMeters());
				parent.ellipse((leg.position.x+1)*wsize.x/2, (leg.position.y+1)*wsize.y/2, sz*wsize.x/2, sz*wsize.y/2);
			}
		}
	}
}

