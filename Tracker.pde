/**
 * oscP5message by andreas schlegel
 * example shows how to create osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */
 
import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;
ArrayList<ParticleSystem> systems;

void setup() {
  size(400,400);
  frameRate(25);
  /* start oscP5, listening for incoming messages from Pulsefield RealTime.m */
  oscP5 = new OscP5(this,7002);
  oscP5.plug(this,"pfframe","/pf/frame");
  oscP5.plug(this,"pfupdate","/pf/update");
  systems = new ArrayList<ParticleSystem>();
}


void draw() {
  background(0);  
  for (ParticleSystem ps: systems) {
    ps.run();
    ps.addParticle();
  }
  if (systems.isEmpty()) {
    fill(255);
    textAlign(CENTER);
    text("Waiting for users...", width/2, height/2);
  }
}

void pfframe(int frame) {
 println("Got frame "+frame);
}

void pfupdate() {
  println("update");
  xpos=200;
  ypos=200;
  id=0;
  systems[id].origin=PVector(xpos,ypos);
  systems.add(new ParticleSystem(1, new PVector(xpos,ypos)));
}
  

/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.isPlugged() == false) {
    print("### Received an unhandled message: ");
    theOscMessage.print();
  }  /* print the address pattern and the typetag of the received OscMessage */
}
