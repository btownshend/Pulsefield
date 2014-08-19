import processing.core.PApplet;
import processing.core.PVector;
import MusicVisualizer.AudioRenderer;
import MusicVisualizer.IsometricRenderer;
import MusicVisualizer.RadarRenderer;
import MusicVisualizer.VortexRenderer;
import ddf.minim.Minim;


public class VisualizerMinim extends VisualizerGrid {
	Minim minim;

	AudioRenderer radar, vortex, iso;
	AudioRenderer[] visuals; 
	int select;
	
	VisualizerMinim(PApplet parent) {
		super(parent);
		select=-1;			// activate first renderer in list
		// setup player
		minim = new Minim(parent);
		// setup renderers
		vortex = new VortexRenderer(parent,minim.getLineIn());
		radar = new RadarRenderer(parent,minim.getLineIn());
		iso = new IsometricRenderer(parent,minim.getLineIn());

		visuals = new AudioRenderer[] {radar, vortex, iso};
	}

	@Override
	public void start(PApplet parent) {
		PApplet.println("Minim.start");
		super.start();
		// Other initialization when this app becomes active
		select=(select+1)%visuals.length;
		visuals[select].start(parent);
		minim.getLineIn().addListener(visuals[select]);
	}

	@Override
	public void stop(PApplet parent) {
		PApplet.println("Minim.stop");
		super.stop();
		minim.getLineIn().removeListener(visuals[select]);
		visuals[select].stop(parent);
	}

	@Override
	public void update(PApplet parent, People p) {
		super.update(parent,p);
//		PApplet.println("Minim.update: minim="+minim+", num people="+p.pmap.size());
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		if (p.pmap.isEmpty()) {
			super.draw(parent, p, wsize);
			return;
		}
		assert(minim!=null);

		// Add drawing code here
		initializeContext(parent);
		visuals[select].draw(parent);
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
		super.drawLaser(parent,p);
	}
}
