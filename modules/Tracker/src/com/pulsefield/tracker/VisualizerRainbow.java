package com.pulsefield.tracker;

import java.awt.Color;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;

public class VisualizerRainbow extends VisualizerParticleSystem {
	private float rainbowPositionDegrees = 0.0f;
	private float rainbowAdvance = 0.0f;
	float rainbowAdvanceIncrement = 0.1f; // rotation of the colors positions
											// per update.
	final boolean useGrid = true;
	Grid grid;

	VisualizerRainbow(PApplet parent) {
		super(parent);
		if (useGrid) {
			final String songs[] = {"OL"};
			grid=new Grid(songs);
		}
	}

	@Override
	void setUniverseDefaults() {
		ParticleSystemSettings pss = new ParticleSystemSettings();
		
		pss.maxParticles = 15000;
		pss.particleScale = 1.0f;
		pss.personForce = 0.002f;
		pss.particleRotation = 0.01f;
		pss.forceRotation = 0.0f;
		pss.particleMaxLife = 175;
		
		universe.settings = pss;
	}
	
	@Override
	public void start() {
		super.start();
		if (useGrid)
			grid.start();
		else
			Ableton.getInstance().setTrackSet("Osmos");
	}

	@Override
	public void stop() {
		super.stop();
		if (useGrid)
			grid.stop();
	}
	
	// Create a particle specified by a distance and angle from a center point.
	private void spewParticle(ParticleSystem universe, int color, float rotation, float angle, float velocity,
			float distanceFromCenter, PVector center) {

		PVector origin = center.copy().add(new PVector(0, distanceFromCenter).rotate(rotation));
		Particle p = new ImageParticle(origin, universe.settings, textures.get("blur.png"));
		p.scale = 0.4f;
		p.color = color;
		
		PVector velocityV = center.copy().sub(origin).normalize().setMag(velocity).rotate(angle);
		p.velocity = velocityV;

		universe.addParticle(p);
	}

	@Override
	public void update(PApplet parent, People p) {
		if (useGrid)
			grid.update(p);
		int particlesPerUpdate = 360;
		rainbowAdvance = (rainbowAdvance + rainbowAdvanceIncrement) % 360;
		
		// Create some particles unless there are too many already.
		if (universe.particlesRemaining() >= 360) {
			for (int i = 0; i < particlesPerUpdate; i++) {
				rainbowPositionDegrees = (rainbowPositionDegrees + 1) % 360;
				int color = Color.HSBtoRGB((rainbowPositionDegrees + rainbowAdvance) / 360.0f, 1.0f, 1.0f);

				spewParticle(universe, color, (rainbowPositionDegrees / 360.0f) * 2 * (float) Math.PI, 0.0f, 0.0f,
						Tracker.getFloorDimensionMin() / 4, Tracker.getFloorCenter());
			}
		}

		applyPeopleGravity(p);
		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (useGrid)
			grid.drawTitle(g);
		drawPeople(g, p);
	}
}
