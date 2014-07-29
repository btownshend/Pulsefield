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
		PGL pgl=((PGraphicsOpenGL)parent.g).pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		super.draw(parent, p, wsize);

		parent.ellipseMode(PConstants.CENTER);
		float sz=20;
		for (Position ps: p.positions.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			parent.ellipse((ps.origin.x+1)*wsize.x/2, (ps.origin.y+1)*wsize.y/2, sz, sz);
		}
	}
}

