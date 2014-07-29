import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;

public class VisualizerTemplate extends Visualizer {

	VisualizerTemplate(PApplet parent) {
		super();
	}

	@Override
	public void start() {
		super.start();
		// Other initialization when this app becomes active
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, Positions p) {
		// Update internal state
	}

	@Override
	public void draw(PApplet parent, Positions p, PVector wsize) {
		super.draw(parent, p, wsize);

		// Add drawing code here
	}
	
	@Override
	public void drawLaser(PApplet parent, Positions p) {
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
