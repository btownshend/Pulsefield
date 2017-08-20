package com.pulsefield.tracker;

import java.awt.Color;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;
import sun.swing.PrintColorUIResource;
import processing.core.PImage;

public class VisualizerRainbow extends VisualizerParticleSystem {
	float personGravity = 0.005f;
	float particleSize = 3.0f;
	PImage image;
	
	float rainbowPositionDegrees = 0.0f;
	
	
	VisualizerRainbow(PApplet parent) {
		super(parent);
		image = parent.loadImage("circle.png");
	}

	
	@Override
	public void start() {
		super.start();
		universe.maxParticles = 20000;
	
		// Rainbow uses legs for attraction scaling.
		Laser.getInstance().setFlag("legs",0.0f);
	}

	
	
	// Create a particle specified by a distance and angle from a center point.
	private void spewParticle(ParticleSystem universe, int color, float rotation, float angle, float velocity,
			float distanceFromCenter,
			PVector center) {
		
		PVector origin = center.copy().add(new PVector(0, distanceFromCenter).rotate(rotation));
		Particle p = new ImageParticle(origin, universe, image);
		p.color = color;

		PVector velocityV = center.copy().sub(origin).normalize().setMag(velocity).rotate(angle);
		p.velocity = velocityV;
		
		universe.addParticle(p);
	}

	@Override
	public void update(PApplet parent, People p) {
		int particlesPerUpdate = 360;

		// Create some particles unless there are too many already.
		if (universe.particlesRemaining() >= 360) {
			for (int i = 0; i < particlesPerUpdate; i++) {
				rainbowPositionDegrees = (rainbowPositionDegrees + 1) % 360;
				int color = Color.HSBtoRGB(rainbowPositionDegrees/360.0f, 1.0f, 1.0f);

				spewParticle(universe, color, (rainbowPositionDegrees / 360.0f) * 2 * (float) Math.PI,
						0.0f, 0.0f, Tracker.getFloorDimensionMin() / 4, Tracker.getFloorCenter());
			}
		}

		applyPeopleGravity(p);
		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		drawPeople(g, p);
	}
}
