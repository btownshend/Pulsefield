package com.pulsefield.tracker;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;
import processing.core.PImage;


public class VisualizerGravity extends Visualizer {

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
			x1 = center.x - (size/2);
			x2 = center.x + (size/2);
			y1 = center.y - (size/2);
			y2 = center.y + (size/2);
		}

		void draw(PGraphics g) {
			g.fill(color);
			g.rect(x1, y1, size, size);

			// Look at the sign of a y coord to determine the "outside" of the field in order
			// to orient the score display.
			int outsidey = (int) Math.signum(y1);
			g.textAlign(PConstants.CENTER,PConstants.CENTER);
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
			if (( p.x > x1 && p.x < x2 ) && (p.y > y1 && p.y < y2)) {
				return true;
			}

			return false;
		}

		void scoreUp() {
			score++;
		}
	}

	GoalBox goal1, goal2, goal3;

	VisualizerGravity(PApplet parent) {
		PImage img = parent.loadImage("circle.png");
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
		Laser.getInstance().setFlag("body",0.0f);
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

	@Override
	public void update(PApplet parent, People p) {

		// TODO: Vary particlesPerUpdate based on sound context.
		int particlesPerUpdate = 15;

		// Create some particles unless there are too many already.
		if (universe.particles.size() < maxParticles) {
			for (int i = 0; i < particlesPerUpdate; i++) {
				// Give particles a dispersing force to mix them up a little
				// (yes, we're re-using this but it's ok).
				PVector acceleration = Particle.genRandomVector(0.01f / 300, 0.0f, 0.0f);

				// Send particles towards the black hole but with a rotation to
				// avoid it.
				PVector corner1 = new PVector(1.0f, -1.0f);
				PVector corner2 = new PVector(-1.0f, 1.0f);
				PVector corner1Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner1)).div(200.0f)
						.rotate(-0.5f);
				PVector corner2Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner2)).div(200.0f)
						.rotate(-0.5f);

				universe.addParticle(Tracker.normalizedToFloor(corner1), corner1Velocity, acceleration, goal1.color);
				universe.addParticle(Tracker.normalizedToFloor(corner2), corner2Velocity, acceleration, goal2.color);

				// Just some fluff particles for more visual interest.
				universe.addParticle(Particle.genRandomVector(Math.max(Tracker.maxx, Tracker.maxy), 0.0f, 0.0f),
						Particle.genRandomVector(0.01f / 300, 0.0f, 0.0f),
						Particle.genRandomVector(0.01f / 300, 0.0f, 0.0f), 0xBBFFFFFF);
			}
		}

		for (int id: p.pmap.keySet()) {
			Person pos=p.pmap.get(id);

			// Users will be half attractors and half repulsors.
			int sign = 1;
			if (id % 2 == 0) {
				sign = -1;
			}

			universe.attractor(pos.getOriginInMeters(), sign * 0.001f);
		}

		// Scan particles for actions needed.
		for (int i = universe.particles.size()-1; i >= 0; i--) {

			// Remove and score particles that have hit a goal.
			// TODO: Refactor for N goals.
			Particle particle = universe.particles.get(i);
			if (goal1.goal(particle)) {
				goal1.scoreUp();
				universe.particles.remove(particle);
			} else if (goal2.goal(particle)) {
				goal2.scoreUp();
				universe.particles.remove(particle);
			}

			// Remove particles that have hit the 'Black hole'.
			if (particle.location.dist(Tracker.getFloorCenter()) < 0.2) {
				universe.particles.remove(particle);
			}

			// Remove particles that have fallen outside the field (with a
			// little buffer).
			float buffer = 1.0f;
			if ((particle.location.x > Tracker.maxx + buffer) || (particle.location.x < Tracker.minx - buffer)
					|| (particle.location.y > Tracker.maxy + buffer) || (particle.location.y < Tracker.miny - buffer)) {

				universe.particles.remove(particle);
			}
		}

		// Central attractor ("black hole").
		universe.attractor(Tracker.getFloorCenter(), 0.005f);

		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		goal1.draw(g);
		goal2.draw(g);

		for (int id: p.pmap.keySet()) {
			String label = "+";
			if (id % 2 == 0) {
				label = "-";
			}

			Person pos=p.pmap.get(id);
			g.fill(0xEEFFFFFF);
			g.textAlign(PConstants.CENTER,PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, label, pos.getOriginInMeters().x, pos.getOriginInMeters().y);
		}

		universe.draw(g);
	}

}
