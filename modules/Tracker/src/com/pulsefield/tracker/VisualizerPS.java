package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;
import processing.opengl.PGL;

// Multiple Particle System class.
public class VisualizerPS extends VisualizerParticleSystem {
	PImage img;
	float attractionForce=1;
	int birthrate=5;
	HashMap<Integer, ParticleSystem> systems;
	
	VisualizerPS(PApplet parent) {
		super(parent);
		systems = new HashMap<Integer,ParticleSystem>();
		img = parent.loadImage("texture.png");
	}
	
	@Override
	public void start() {
		super.start();
	}
	
	@Override
	void setUniverseDefaults() {
		universe.tilty = -0.03f / 300;
		universe.particleMaxLife = 500;
		universe.startOpacity = 0.05f;
		universe.forceRotation = 90;
		universe.particleRandomDriftAccel = 0.0f;
		universe.blendMode = -1; // Use custom blend.
		universe.personForce = 0f;
	}

	public void update(PApplet parent, People p) {
		for (int id: p.pmap.keySet()) {
			Person pos=p.pmap.get(id);
			ParticleSystem ps=systems.get(id);			
			if (ps==null) {
				logger.fine("Added new particle system for ID "+id);
				ps=new ParticleSystem();
				systems.put(id,ps);
				
				PApplet.println("Created new ParticleSystem for user " + pos);
			}

			// Push the other systems with each positions velocity
			if (pos.isMoving())
				for (ParticleSystem ps2: systems.values()) {
					//ps.attractor(ps2.origin, attractionForce);
					//					logger.fine("Pushing PS "+ps2+" from "+pos.getNormalizedPosition()+" with vel="+pos.getNormalizedVelocity());
					ps2.push(pos.getOriginInMeters(), PVector.div(pos.getVelocityInMeters(),parent.frameRate));
				}

			for (int k=0;k<birthrate;k++) {
				Particle particle = new ImageParticle(pos.getOriginInMeters(), ps, img);
				
				particle.color = pos.getcolor();
				particle.velocity = Particle.fuzzVector(new PVector(0f,0f), 0.3f/300, 0.3f/300);
				particle.adoptSettingsFromParticleSystem(universe);
				
				ps.addParticle(particle);
			}
		}
		int toRemove=-1;

		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			ps.update();
			
			if (universe.personForce != 0f) {
				applyPeopleGravity(ps, p, universe.personForce);
			}

			int id = me.getKey();
			if (ps.dead()) {
				logger.fine("ID "+id+" is dead.");
				toRemove=id;
			}
		}
		if (toRemove!=-1) {
			systems.remove(toRemove);
			logger.fine("Removed system for ID "+toRemove);
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t,g,p);
		if (p.pmap.isEmpty())
			return;
		
		for (Map.Entry<Integer,ParticleSystem> me: systems.entrySet()) {
			ParticleSystem ps=me.getValue();
			ps.draw(g);
		}
	}

}

