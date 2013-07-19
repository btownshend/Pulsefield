import processing.core.PApplet;

public class Tracker extends PApplet {
	/**
	 * 
	 */
	private static boolean present = false;

	private static final long serialVersionUID = 1L;
	int tick=0;
	Pulsefield pf;
	private float avgFrameRate=0;
	
	public void setup() {
		size(1280,800, OPENGL);
		frameRate(30);
		pf = new PulsefieldPS(this);
	}

	public void draw() {
		background(0);
		tick++;
		avgFrameRate=avgFrameRate*(1f-1f/200f)+frameRate/200f;
		if (tick%200 == 0)
			println("Average frame rate = "+avgFrameRate);
			
				
		if (mousePressed)
			pf.pfupdate(tick/avgFrameRate,98, mouseX, mouseY);

		pf.draw();
		
	}

	public void mouseReleased() {
		pf.pfexit(0, 0, 98);
	}
	
	public static void main(String args[]) {
		if (present)
		PApplet.main(new String[] { "--present","Tracker" });
		else
			PApplet.main(new String[] { "Tracker" });
		
	}
}

