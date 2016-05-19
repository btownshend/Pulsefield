import processing.core.PApplet;
import processing.core.PGraphics;

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
	public void update(PApplet parent, People p) {
		// Update internal state
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		// Add drawing code here
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
