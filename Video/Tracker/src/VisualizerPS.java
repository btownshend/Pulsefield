import java.util.HashMap;
import java.util.Map;

import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;
import processing.opengl.PGL;

public class VisualizerPS extends Visualizer {
	PImage img;
	float attractionForce=1;
	int birthrate=5;
	HashMap<Integer, ParticleSystem> systems;

	VisualizerPS(PApplet parent) {
		super();
		systems = new HashMap<Integer,ParticleSystem>();
		img = parent.loadImage("texture.png");
	}

	public void add(int id, int channel) {
		ParticleSystem ps=new ParticleSystem(new PVector(0f,0f),channel, img);
		systems.put(id,ps);
	}

	public void setnpeople(int n) {
		if (n!=systems.size()) {
			PApplet.println("Have "+systems.size()+" people, but got message that there are "+n+" .. clearing.");
			systems.clear();
		}
	}
	
	public void move(int id, int channel, PVector newpos, float elapsed) {
		ParticleSystem ps=systems.get(id);
		if (ps==null) {
			PApplet.println("Unable to locate user "+id+", creating it.");
			add(id,channel);
			ps=systems.get(id);
		}
		ps.move(newpos,elapsed);
		if (!ps.enabled) {
			PApplet.println("Enabling ID "+id);
			ps.enable(true);
		}
	}

	public void update(PApplet parent) {
		int toRemove=-1;

		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			int id = me.getKey();
			ps.update();
			if (ps.enabled) {
				for (int k=0;k<birthrate;k++)
					ps.addParticle();
			}
			else if (ps.dead()) {
				PApplet.println("ID "+id+" is dead.");
				toRemove=id;
			}
			// Push the other systems with each positions velocity
			for (Position ps2: systems.values()) {
				// if (ps!=ps2) 
				//ps.attractor(ps2.origin, attractionForce);
				ps.push(ps2.origin, PVector.div(ps2.avgspeed,parent.frameRate));   // Convert velocity into pixels/frame
			}
		}
		if (toRemove!=-1) {
			systems.remove(toRemove);
			PApplet.println("Removed system for ID "+toRemove);
		}
	}

	public void draw(PApplet parent) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			ps.draw(parent);
		}

		if (systems.isEmpty()) {
			parent.fill(50, 255, 255);
			parent.textAlign(PConstants.CENTER);
			parent.textSize(32);
			parent.text("Waiting for users...", parent.width/2, parent.height/2);
		}
	}

	public void exit(int id) {
		if (systems.containsKey(id)) {
			systems.get(id).enable(false);
		} 
		else
			PApplet.println("Unable to locate particle system "+id);
	}

	public void clear() {
		systems.clear();
	}
}

