package com.pulsefield.tracker;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

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

		// Look at the sign of a y coord to determine the "outside" of the field
		// in order
		// to orient the score display.
		int outsidey = (int) Math.signum(y1);
		g.textAlign(PConstants.CENTER, PConstants.CENTER);
		Visualizer.drawText(g, 0.3f, Integer.toString(score), center.x, center.y + (-0.8f * outsidey * size));
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

public class VisualizerGravity extends VisualizerParticleSystem {
	GoalBox goal1, goal2;
	int winningScore = 10000;
	boolean gameOver = false;
	int updatesBetweenNextGame = 600; // Needs to be longer than
	// maxParticleLife.
	int intergameUpdateCounter = 0;

	VisualizerGravity(PApplet parent) {
		super(parent);

		// Create Goals.
		goal1 = new GoalBox();
		goal2 = new GoalBox();
	}

	@Override
	public void start() {
		super.start();

		Ableton.getInstance().setTrackSet("Osmos");

		// Reset goal scores.
		goal1.score = 0;
		goal2.score = 0;
		
		PVector p1 = Tracker.normalizedToFloor(new PVector(-0.8f, -0.8f));
		PVector p2 = Tracker.normalizedToFloor(new PVector(0.8f, 0.8f));
		goal1.setCenter(p1);
		goal1.color = 0xFFDD22DD;
		goal2.setCenter(p2);
		goal2.color = 0xFF22DDDD;
	}

	void resetGame() {
		gameOver = false;
		goal1.score = 0;
		goal2.score = 0;
	}

	
	void setUniverseDefaults() {
		ParticleSystemSettings pss = new ParticleSystemSettings();
		
		pss.maxParticles = 20000;
		pss.particleRandomDriftAccel = 0.000008f;
		pss.particleMaxLife = 500;
		pss.forceRotation = 340.0f;
		pss.particleScale = 0.5f;
		pss.personForce = 0.001f;
		
		universe.settings = pss;
	}

	@Override
	public void update(PApplet parent, People p) {
		// Reset the game when it's time.
		if (gameOver) {
			if (intergameUpdateCounter-- <= 0) {
				resetGame();
			}
		} else {
			// Resize goals for field.
			goal1.setSize(Tracker.getFloorDimensionMin() / 8);
			goal2.setSize(Tracker.getFloorDimensionMin() / 8);
		}

		// TODO: Vary particlesPerUpdate based on sound context.
		// TODO: OSC Control for this.
		int particlesPerUpdate = 10;

		// Create some particles unless there are too many already.
		if (universe.particlesRemaining() > particlesPerUpdate) {
			for (int i = 0; i < particlesPerUpdate; i++) {

				if (!gameOver) {
					// Send particles towards the black hole but with a
					// rotation to avoid going directly in.
					PVector corner1 = Tracker.normalizedToFloor(new PVector(1.0f, -1.0f));
					PVector corner2 = Tracker.normalizedToFloor(new PVector(-1.0f, 1.0f));
					PVector corner1Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner1))
							.normalize().div(30).rotate(-0.5f);
					PVector corner2Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner2))
							.normalize().div(30).rotate(-0.5f);

					Particle particle1 = new ImageParticle(corner1, universe.settings, textures.get("circle.png"));
					particle1.velocity = corner1Velocity;
					particle1.color = goal1.color;
					particle1.rotationRadians = 0.0f;

					Particle particle2 = new ImageParticle(corner2, universe.settings, textures.get("circle.png"));
					particle2.velocity = corner2Velocity;
					particle2.color = goal2.color;
					particle2.rotationRadians = 0.0f;

					universe.addParticle(particle1);
					universe.addParticle(particle2);
				}

				// Just some fluff particles for more visual interest.
				Particle particleFluff = new ImageParticle(
						Particle.genRandomVector(Tracker.getFloorDimensionMax() / 2, 0f, 0f), universe.settings,
						textures.get("blur.png"));
				particleFluff.velocity = Particle.genRandomVector(0.005f / 300, 0.0f, 0.0f);
				particleFluff.color = 0x99FFFFFF;
				particleFluff.scale = 0.5f;
				particleFluff.maxLifespan = 200;
				particleFluff.opacity = 0.4f;

				if (gameOver && intergameUpdateCounter > 300) {
					particleFluff.scale = 0.8f;
					particleFluff.opacity = 0.8f;

					// Use color + 1 so fluff doesn't get scored when the
					// game restarts.
					if (goal1.score >= winningScore) {
						particleFluff.color = goal1.color + 1;
						universe.addParticle(particleFluff);
					}
					if (goal2.score >= winningScore) {
						particleFluff.color = goal2.color + 1;
						universe.addParticle(particleFluff);
					}
				} else {
					universe.addParticle(particleFluff);
				}
			}
		}

		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);

			// Users will be half attractors and half repulsors.
			int sign = 1;
			if (id % 2 == 0) {
				sign = -1;
			}

			// Increase gravity if a person is moving making dynamic
			// solutions to the game interesting.
			universe.attractor(pos.getOriginInMeters(), sign * universe.settings.personForce
					* (float) Math.min(Math.max(pos.getVelocityInMeters().mag() * 3.0f, 1.0), 2.0));
		}

		// Scan particles for actions needed.
		for (int i = universe.particles.size() - 1; i >= 0; i--) {

			// Remove and score particles that have hit a goal.
			// TODO: Refactor for N goals.
			Particle particle = universe.particles.get(i);

			if (!gameOver) {
				if (goal1.goal(particle)) {
					goal1.scoreUp();
					universe.particles.remove(particle);
				} else if (goal2.goal(particle)) {
					goal2.scoreUp();
					universe.particles.remove(particle);
				}

				if (goal1.score >= winningScore) {
					gameOver = true;
					goal1.setSize(Tracker.getFloorDimensionMin() / 6);
					intergameUpdateCounter = updatesBetweenNextGame;
				}
				if (goal2.score >= winningScore) {
					gameOver = true;
					goal2.setSize(Tracker.getFloorDimensionMin() / 6);
					intergameUpdateCounter = updatesBetweenNextGame;
				}
			}

			// Remove particles that have hit the 'Black hole'.
			if (particle.location.dist(Tracker.getFloorCenter()) < 0.2) {
				universe.particles.remove(particle);
			}
		}

		// Central attractor ("black hole").
		// TODO: OSC Slider for black hole force.
		universe.attractor(Tracker.getFloorCenter(), 0.006f);

		// Clear the field before the next game starts.
		if (intergameUpdateCounter > 0 && intergameUpdateCounter < 300) {
			universe.attractor(Tracker.getFloorCenter(), 0.0001f * (300 - intergameUpdateCounter));
		}

		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		PVector p1 = Tracker.normalizedToFloor(new PVector(-0.8f, -0.8f));
		PVector p2 = Tracker.normalizedToFloor(new PVector(0.8f, 0.8f));
		goal1.setCenter(p1);
		goal2.setCenter(p2);
		
		goal1.draw(g);
		goal2.draw(g);

		for (int id : p.pmap.keySet()) {
			String label = "+";
			if (id % 2 == 0) {
				label = "-";
			}

			Person pos = p.pmap.get(id);
			g.fill(0xEEFFFFFF);
			g.textAlign(PConstants.CENTER, PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, label, pos.getOriginInMeters().x, pos.getOriginInMeters().y);
		}
	}
}

