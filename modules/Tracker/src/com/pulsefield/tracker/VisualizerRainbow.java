package com.pulsefield.tracker;

import java.awt.Color;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;
import processing.core.PImage;

public class VisualizerRainbow extends Visualizer {

	ParticleSystem universe;
	int maxParticles = 30000;

	class GoalBox {
		Integer color = 0xFFDDDDDD;
		Integer score = 0;

		Float size = 0.2f;

		private PVector center; // Center coordinate.
		private Float x1, x2, y1, y2;

		void setCenter(PVector c) {
			center = c;
			calcCoords();
		}

		void setSize(Float s) {
			size = s;
			calcCoords();
		}

		private void calcCoords() {
			x1 = center.x - (size / 2);
			x2 = center.x + (size / 2);
			y1 = center.y - (size / 2);
			y2 = center.y + (size / 2);
		}

		void draw(PGraphics g) {
			g.fill(color);
			g.rect(x1, y1, size, size);

			// Look at the sign of a y coord to determine the "outside" of the
			// field in order
			// to orient the score display.
			int outsidey = (int) Math.signum(y1);
			g.textAlign(PConstants.CENTER, PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, Integer.toString(score), center.x, center.y + (-1 * outsidey * size));
		}

		// Particle goal check includes color matching.
		Boolean goal(Particle p) {
			if (p.color != color)
				return false;

			return goal(p.location);
		}

		// Check if the given vector is inside the goalbox.
		Boolean goal(PVector p) {
			if ((p.x > x1 && p.x < x2) && (p.y > y1 && p.y < y2)) {
				return true;
			}

			return false;
		}

		void scoreUp() {
			score++;
		}
	}

	GoalBox goal1, goal2, goal3;

	VisualizerRainbow(PApplet parent) {
		PImage img = parent.loadImage("3x3-solid.png");
		universe = new ParticleSystem(img);

		// Create Goals.
		goal1 = new GoalBox();
		goal2 = new GoalBox();
		PVector p1 = Tracker.normalizedToFloor(new PVector(-0.8f, -0.8f));
		PVector p2 = Tracker.normalizedToFloor(new PVector(0.8f, 0.8f));
		goal1.setCenter(p1);
		goal1.color = 0xFFDD22DD;
		goal2.setCenter(p2);
		goal2.color = 0xFF22DDDD;

	}

	@Override
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body", 0.0f);
		// Laser.getInstance().setFlag("legs",0.0f);

		// Reset goal scores.
		goal1.score = 0;
		goal2.score = 0;

		// Resize goals for field.
		goal1.setSize(Tracker.getFloorDimensionMin() / 8);
		goal2.setSize(Tracker.getFloorDimensionMin() / 8);
	}

	@Override
	public void stop() {
		super.stop();
	}

	private void spewParticle(ParticleSystem universe, int color, float rotation, float angle, float velocity,
			float distanceFromCenter,
			PVector center) {

		PVector acceleration = Particle.genRandomVector(0.04f / 300, 0.0f, 0.0f);
		PVector origin = center.copy().add(new PVector(0, distanceFromCenter).rotate(rotation));
		PVector velocityV = center.copy().sub(origin).normalize().setMag(velocity).rotate(angle);

		universe.addParticle(origin, velocityV, acceleration, color);
	}

	@Override
	public void update(PApplet parent, People p) {

		// TODO: Vary particlesPerUpdate based on sound context.
		int particlesPerUpdate = 1;

		// Create some particles unless there are too many already.
		if (universe.particles.size() < maxParticles) {

			for (int i = 0; i < particlesPerUpdate; i++) {
				for (int j = 0; j < 360; j += 2) {
					int color = Color.HSBtoRGB(j/360.0f, 1.0f, 1.0f);

					spewParticle(universe, color, (j / 360.0f) * 2 * (float) Math.PI,
							0.0f, 0.0f, Tracker.getFloorDimensionMin() / 4, Tracker.getFloorCenter());
				}
			}
		}

		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);

			universe.attractor(pos.getOriginInMeters(), 0.005f * (1 - pos.getLegSeparationInMeters()));
		}

		// Scan particles for actions needed.
		for (int i = universe.particles.size() - 1; i >= 0; i--) {
			Particle particle = universe.particles.get(i);

			// Remove particles that have fallen outside the field (with a
			// little buffer).
			float buffer = 0.1f;
			if ((particle.location.x > Tracker.maxx + buffer) || (particle.location.x < Tracker.minx - buffer)
					|| (particle.location.y > Tracker.maxy + buffer) || (particle.location.y < Tracker.miny - buffer)) {

				universe.particles.remove(particle);
			}
		}

		// Central attractor ("black hole").
		// universe.attractor(Tracker.getFloorCenter(), 0.005f);

		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		// goal1.draw(g);
		// goal2.draw(g);

		for (int id : p.pmap.keySet()) {
			String label = "+";

			Person pos = p.pmap.get(id);
			g.fill(0xEEFFFFFF);
			g.textAlign(PConstants.CENTER, PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, label, pos.getOriginInMeters().x, pos.getOriginInMeters().y);
		}

		universe.draw(g);
	}

}
