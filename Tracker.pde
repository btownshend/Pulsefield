/**
 * oscP5message by andreas schlegel
 * example shows how to create osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */

float attractionForce=1;
int birthrate=5;
float tick=0;
PulsefieldPS pf;


void setup() {
  size(1280, 800, OPENGL);
  frameRate(60);
  pf = new PulsefieldPS();
}


void draw() {
  tick+=1/frameRate;
  PGL pgl=((PGraphicsOpenGL)g).pgl;
  pgl.blendFunc(pgl.SRC_ALPHA, pgl.DST_ALPHA);
  pgl.blendEquation(pgl.FUNC_ADD);  
  background(0, 0, 0);  
  colorMode(RGB, 255);

  if (mousePressed)
    pf.pfupdate(tick,98, mouseX, mouseY);

  pf.draw();
}

void mouseReleased() {
  pf.pfexit(0, 0, 98);
}


