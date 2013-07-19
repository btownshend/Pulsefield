/**
 * oscP5message by andreas schlegel
 * example shows how to create osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */

float attractionForce=1;
int birthrate=5;
float tick=0;
Pulsefield pf;


void setup() {
	size(400,400, OPENGL);
	frameRate(30);
	pf = new PulsefieldNavier();
}


void draw() {
	tick+=1/frameRate;

	if (mousePressed)
		pf.pfupdate(tick,98, mouseX, mouseY);

	pf.draw();
}

void mouseReleased() {
	pf.pfexit(0, 0, 98);
}


