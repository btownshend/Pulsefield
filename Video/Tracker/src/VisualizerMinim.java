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
	final int numRenderers=3;
	
	VisualizerMinim(PApplet parent) {
		select=0;			// activate first renderer in list
		minim=null; 	// Will be initialized in update() when someone enters
		super(parent);
	}

	@Override
	public void start(PApplet parent) {
		PApplet.println("Minim.start");
		super.start();
		// Other initialization when this app becomes active
		select=(select+1)%numRenderers;
		visuals[select].start(parent);
	}

	@Override
	public void stop(PApplet parent) {
		PApplet.println("Minim.stop");
		super.stop();
		// When this app is deactivated
//		groove.close();
		if (minim!=null) {
			minim.getLineIn().removeListener(visuals[select]);
			minim.stop();	
			minim=null;
		}
		visuals[select].stop(parent);
	}

	@Override
	public void update(PApplet parent, People p) {
		super.update(parent,p);
//		PApplet.println("Minim.update: minim="+minim+", num people="+p.pmap.size());
		if (p.pmap.isEmpty() && minim!=null) {
			minim.stop();
			minim=null;
		} else if (!p.pmap.isEmpty() && minim==null) {
			// setup player
			minim = new Minim(parent);
			// setup renderers
			vortex = new VortexRenderer(parent,minim.getLineIn());
			radar = new RadarRenderer(parent,minim.getLineIn());
			iso = new IsometricRenderer(parent,minim.getLineIn());

			visuals = new AudioRenderer[] {radar, vortex, iso};
			visuals[select].setup(parent);
			minim.getLineIn().addListener(visuals[select]);
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		if (minim==null || p.pmap.isEmpty()) {
			super.draw(parent, p, wsize);
			return;
		}
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
