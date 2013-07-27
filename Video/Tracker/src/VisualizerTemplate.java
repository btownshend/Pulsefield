import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;

public class VisualizerTemplate extends Visualizer {

	VisualizerTemplate(PApplet parent) {
		super();
	}

	public void update(PApplet parent, Positions p) {
		// Update internal state
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		super.draw(parent, p, wsize);

		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0);
		drawBorders(parent,true,wsize);

		// Add drawing code here
	}
}

