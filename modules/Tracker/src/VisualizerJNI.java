import processing.core.PApplet;
import processing.core.PGraphics;

public class VisualizerJNI extends Visualizer {
	private native void nsetup(PApplet parent);
	private native void  nstart();
	private native void  nstop();
	private native void nupdate(PApplet parent, People p);
	private native void ndraw(Tracker t, PGraphics g, People p);
	
    static {
    	String jlp=System.getProperty("java.library.path");
    	System.out.println("java.library.path="+jlp);
        System.loadLibrary("exampleLibrary");
    }
    
	VisualizerJNI(PApplet parent) {
		super();
		nsetup(parent);
	}
	
	@Override
	public void start() {
		super.start();
		nstart();
	}
	
	@Override
	public void stop() {
		super.stop();
		nstop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
		nupdate(parent,p);
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		//super.draw(t, g, p);
		ndraw(t,g,p);
		// Add drawing code here
	}
}
