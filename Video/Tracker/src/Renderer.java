import processing.core.PApplet;


public abstract class Renderer {
	Fourier fourier;
	MusicVisLaser mvl;
	
	Renderer(Fourier f, MusicVisLaser.Modes laserMode) {
		fourier=f;
		mvl=new MusicVisLaser(fourier, laserMode);
	}
	public void start() {}
	public void stop() {}
	public abstract void draw(PApplet parent); 
	public void update(PApplet parent) { }
	public  void drawLaser(PApplet parent, People p) {
		// Delegate to the mvl
		mvl.drawLaser(parent, p);
	}
}
