import processing.core.PApplet;
import processing.core.PGraphics;


public abstract class Renderer {
	Fourier fourier;
	MusicVisLaser mvl;
	
	Renderer(Fourier f, MusicVisLaser.Modes laserMode) {
		fourier=f;
		mvl=new MusicVisLaser(fourier, laserMode);
	}
	public void start() {}
	public void stop() {}
	public abstract void draw(Tracker tracker, PGraphics g); 
	public void update(PApplet parent) { }
	public  void drawLaser(PApplet parent, People p) {
		// Delegate to the mvl
		mvl.drawLaser(parent, p);
	}

}
