import java.util.Map;

import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;
import processing.opengl.PGL;

class PulsefieldPS extends Pulsefield {
	PImage img;
	float attractionForce=1;
	int birthrate=5;

	PulsefieldPS(PApplet parent) {
		super(parent);
		img = parent.loadImage("texture.png");
	}
	
	synchronized Position add(int id, int channel) {
		int col=parent.color((channel*37)%255, (channel*91)%255, (channel*211)%255);
		// col=color(255,255,255);
		PApplet.println("Color="+col);

		
		Position ps=new ParticleSystem(parent, 0, mapposition(maxx, 0), col, img);
		positions.put(id,ps);

		return ps;
	}


	synchronized void draw() {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		int toRemove=-1;
		for (Map.Entry<Integer,Position> me: positions.entrySet()) {
			ParticleSystem ps=(ParticleSystem)me.getValue();
			int id = me.getKey();
					
			// Push the other systems with each positions velocity
			for (Position ps2: positions.values()) {
				// if (ps!=ps2) 
				//ps.attractor(ps2.origin, attractionForce);
				ps.push(ps2.origin, PVector.div(ps2.avgspeed,parent.frameRate));   // Convert velocity into pixels/frame
			}

			ps.run();
			if (ps.enabled) {
				for (int k=0;k<birthrate;k++)
					ps.addParticle();
			}
			else if (ps.dead()) {
				PApplet.println("ID "+id+" is dead.");
				toRemove=id;
			}
		}
		if (toRemove!=-1) {
			positions.remove(toRemove);
			PApplet.println("Removed system for ID "+toRemove);
		}
		if (positions.isEmpty()) {
			parent.fill(50, 255, 255);
			parent.textAlign(PConstants.CENTER);
			parent.textSize(32);
			parent.text("Waiting for users...", parent.width/2, parent.height/2);
		}
	}




}

