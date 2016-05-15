import java.util.HashMap;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PImage;
import processing.core.PVector;
import processing.core.PConstants;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

public class VisualizerPS extends Visualizer {
	PImage img;
	float attractionForce=1;
	int birthrate=20;
	HashMap<Integer, ParticleSystem> systems;

	VisualizerPS(PApplet parent) {
		super();
		systems = new HashMap<Integer,ParticleSystem>();
		img = parent.loadImage("texture.png");
	}

	public void update(PApplet parent, People p) {
		for (int id: p.pmap.keySet()) {
			Person pos=p.pmap.get(id);
			ParticleSystem ps=systems.get(id);
			if (ps==null) {
				PApplet.println("Added new particle system for ID "+id);
				ps=new ParticleSystem(img);
				systems.put(id,ps);
			}

			// Push the other systems with each positions velocity
			if (pos.isMoving())
				for (ParticleSystem ps2: systems.values()) {
					// if (ps!=ps2) 
					//ps.attractor(ps2.origin, attractionForce);
//					PApplet.println("Pushing PS "+ps2+" from "+pos.getNormalizedPosition()+" with vel="+pos.getNormalizedVelocity());
					ps2.push(pos.getNormalizedPosition(), PVector.div(pos.getNormalizedVelocity(),parent.frameRate));   // Convert velocity into normalized units/frame
				}

			for (int k=0;k<birthrate;k++)
				ps.addParticle(pos.getNormalizedPosition(),pos.getcolor(parent));
		}
		int toRemove=-1;

		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			ps.update();

			int id = me.getKey();
			if (ps.dead()) {
				PApplet.println("ID "+id+" is dead.");
				toRemove=id;
			}
		}
		if (toRemove!=-1) {
			systems.remove(toRemove);
			PApplet.println("Removed system for ID "+toRemove);
		}
	}

	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent,p,wsize);
		if (p.pmap.isEmpty())
			return;
		if (true) {
			// The coercion to PGraphicsOpenGL doesn't work when using FX2D
			// In any case, even with P2D, I don't see that this makes any difference
			// But running parent.blendMode(PConstants.ADD) looks worse...
			parent.resetShader();
			PGL pgl=parent.beginPGL();
			pgl.blendFunc(PGL.SRC_ALPHA, PGL.ONE_MINUS_SRC_ALPHA); 
			pgl.blendEquation(PGL.FUNC_ADD);
			parent.endPGL();
		}

		
		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			ps.draw(parent, wsize);
		}
	}

}

