import processing.core.PApplet;
import processing.core.PVector;
import MusicVisualizer.AudioRenderer;
import MusicVisualizer.IsometricRenderer;
import MusicVisualizer.RadarRenderer;
import ddf.minim.AudioPlayer;
import ddf.minim.Minim;


public class VisualizerMinim extends Visualizer {
	Minim minim;

	AudioRenderer radar, vortex, iso;
	AudioRenderer[] visuals; 
	int select;

	VisualizerMinim(PApplet parent) {
		super();

		// setup renderers
		//		vortex = new VortexRenderer(groove);
		radar = new RadarRenderer(parent,minim.getLineIn());
		iso = new IsometricRenderer(parent,minim.getLineIn());

		visuals = new AudioRenderer[] {radar, /*vortex, */ iso};
		
		select=0;			// activate first renderer in list
	}

	@Override
	public void start() {
		super.start();
		// Other initialization when this app becomes active
		select=(select+1)%visuals.length;
	}

	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
//		groove.close();
		if (minim != null)
			minim.stop();	
	}

	@Override
	public void update(PApplet parent, People p) {
		if (p.pmap.isEmpty()) {
			minim.stop();
			minim=null;
		} else if (minim==null) {
			// setup player
			minim = new Minim(parent);
			minim.getLineIn().addListener(visuals[select]);
			visuals[select].setup(parent);
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		if (minim==null) {
			super.draw(parent, p, wsize);
			return;
		}
		// Add drawing code here
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
	}
}
