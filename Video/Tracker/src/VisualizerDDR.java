import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;

// Dance revolution visualizer
public class VisualizerDDR extends Visualizer {

	VisualizerDDR(PApplet parent) {
		super();
	}

	public void update(PApplet parent, Positions p) {
		// Update internal state
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		final float sidesize=(wsize.x-wsize.y)/2;
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);
		super.draw(parent, p, wsize);


		drawScores(parent,new PVector(sidesize,wsize.y));
		parent.translate(sidesize,0);
		drawPF(parent,p,new PVector(wsize.x-2*sidesize,wsize.y));
		parent.translate(wsize.x-sidesize,0);
		drawTicker(parent,new PVector(sidesize,wsize.y));
	}

	public void drawPF(PApplet parent, Positions p, PVector wsize) {
		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0);
		drawBorders(parent,true,wsize);

		// Add drawing code here
	}

	public void drawScores(PApplet parent, PVector wsize) {
	}

	public void drawTicker(PApplet parent, PVector wsize) {
	}
}

