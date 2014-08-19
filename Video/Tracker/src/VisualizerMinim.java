import processing.core.PApplet;
import processing.core.PVector;
import ddf.minim.Minim;


public class VisualizerMinim extends VisualizerGrid {
	Minim minim;

	Renderer radar, vortex, iso;
	Renderer[] visuals; 
	Fourier fourier;
	int select;
	
	VisualizerMinim(PApplet parent) {
		super(parent);
		select=-1;			// activate first renderer in list
		// setup player
		minim = new Minim(parent);
		// setup renderers
		fourier=new Fourier(minim.getLineIn());
		minim.getLineIn().addListener(fourier);
		vortex = new VortexRenderer(fourier);
		radar = new RadarRenderer(fourier);
		iso = new IsometricRenderer(parent,fourier);

		visuals = new Renderer[] {radar, vortex, iso};
	}

	@Override
	public void start() {
		PApplet.println("Minim.start");
		super.start();
		// Other initialization when this app becomes active
		select=(select+1)%visuals.length;
		visuals[select].start();
	}

	@Override
	public void stop() {
		PApplet.println("Minim.stop");
		super.stop();
		visuals[select].stop();
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
		Laser laser=Laser.getInstance();
		for (Person ps: p.pmap.values()) {  
			laser.cellBegin(ps.id); // Start a cell-specific drawing
			visuals[select].drawLaserPerson(parent, ps.id);
			laser.cellEnd(ps.id);
		}
	}
}
