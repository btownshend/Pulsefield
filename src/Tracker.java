/**
 * oscP5message by andreas schlegel
 * example shows how to create osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */

float attractionForce=1;
int birthrate=5;
float tick=0;


	size(400,400, OPENGL);
	pf = new PulsefieldNavier();
	Pulsefield pf;
	public void setup() {
		frameRate(30);
	public void draw() {
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


	tick+=1/frameRate;

