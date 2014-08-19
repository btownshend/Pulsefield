import processing.core.PApplet;
import processing.core.PVector;



public class VisualizerMinim extends VisualizerGrid {
	Renderer radar, vortex, iso;
	Renderer[] visuals; 
	Fourier fourier;
	int select;
	
	VisualizerMinim(PApplet parent, Fourier fourier) {
		super(parent);

		this.fourier=fourier;
		select=-1;			// activate first renderer in list
		// setup renderers
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
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
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
		visuals[select].update(parent);
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		if (p.pmap.isEmpty()) {
			super.draw(parent, p, wsize);
			return;
		}

		// Add drawing code here
		initializeContext(parent);
		visuals[select].draw(parent);
	}

	@Override
	public void drawLaser(PApplet parent, People p) {
		visuals[select].drawLaser(parent, p);
	}
}
