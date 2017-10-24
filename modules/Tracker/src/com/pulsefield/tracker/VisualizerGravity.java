package com.pulsefield.tracker;

import java.awt.Color;
import java.util.Random;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

class GoalBox {
	Integer color = 0xFFDDDDDD;
	Integer score = 0;
	boolean enabled = true;

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
		// Look at the sign of a y coord to determine the "outside" of the field
		// in order to orient the score display.
		int outsidey = (int) Math.signum(y1);
		g.textAlign(PConstants.CENTER, PConstants.CENTER);
		g.fill(color);
		Visualizer.drawText(g, 0.3f, Integer.toString(score), center.x, center.y + (-0.8f * outsidey * size));

		g.rect(x1, y1, size, size);
	}

	// Particle goal check includes color matching.
	Boolean goal(Particle p) {
		if (p.color != color)
			return false;

		return goal(p.location);
	}

	// Check if the given vector is inside the goalbox.
	Boolean goal(PVector p) {
		if (!enabled)
			return false;

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
	int winningScore = 3000;
	boolean gameOver = false;
	int updatesBetweenNextGame = 600; // Count of updates after 'win' until restart.
	int intergameUpdateCounter = 0;
	int seed = 0;

	static Random rng = new Random();
	int color1, color2;

	// teamAssignment acts to randomize team allocation (color).
	boolean teamAssignment[] = new boolean[100];

	// Rotating center image representing the blackhole.
	Particle blackhole;

	@SuppressWarnings("unused")
	private final static Logger logger = Logger.getLogger(Tracker.class.getName());
	
	float sourceVelocity;
	float sourceAngle;

	VisualizerGravity(PApplet parent) {
		super(parent);

		// Create Goals.
		goal1 = new GoalBox();
		goal2 = new GoalBox();
	}

	@Override
	public void start() {
		super.start();

		// Rotating black hole image.
		PImage blackholeImage = textures.get("blackhole.png");
		ParticleSystemSettings blackholePSS = new ParticleSystemSettings();
		blackholePSS.particleRotation = 0.1f;
		blackhole = new ImageParticle(Tracker.getFloorCenter(), blackholePSS, blackholeImage);
		blackhole.rotationRate = 0.01f;
		blackhole.scale = 0.6f;
		blackhole.setLifespan(-1);

		resetGame();
	}

	void resetGame() {
		gameOver = false;
		goal1.score = 0;
		goal2.score = 0;

		// Used to randomize person color assignments.
		for (int i = 0; i < teamAssignment.length; i++) {
			teamAssignment[i] = rng.nextBoolean();
		}

		// Choose colors for game.
		seed = rng.nextInt();
		int colordegree = rng.nextInt() % 360;
		color1 = Color.HSBtoRGB(colordegree / 360.0f, 1.0f, 1.0f);
		color2 = Color.HSBtoRGB((colordegree + 90) / 360.0f, 1.0f, 1.0f);
		goal1.color = color1;
		goal2.color = color2;
		
		// Set some randomized game parameters.
		sourceVelocity = (float) (rng.nextFloat() * 0.02 + 0.001);
		sourceAngle = (float) (rng.nextFloat()) * 2 - 1.0f;
	}

	void setUniverseDefaults() {
		ParticleSystemSettings pss = new ParticleSystemSettings();
		
		pss.maxParticles = 10000;
		pss.particleRandomDriftAccel = 0.000006f;
		pss.particleMaxLife = 350;
		pss.forceRotation = 0.0f;
		pss.particleScale = rng.nextFloat() * 0.6f + 0.25f ;  // Good static value is 0.45f;
		pss.personForce = 0.0025f;
		
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

		// Place goals.
		PVector p1 = Tracker.normalizedToFloor(new PVector(-0.8f, -0.8f));
		PVector p2 = Tracker.normalizedToFloor(new PVector(0.8f, 0.8f));
		goal1.setCenter(p1);
		goal2.setCenter(p2);

		// Update blackhole for rotation and force to center.
		blackhole.update();
		blackhole.location = Tracker.getFloorCenter();

		// TODO: Vary particlesPerUpdate based on sound context.
		// TODO: OSC Control for this.
		int particlesPerUpdate = 5;

		// Create some particles unless there are too many already.
		if (universe.particlesRemaining() > particlesPerUpdate) {
			for (int i = 0; i < particlesPerUpdate; i++) {

				if (!gameOver) {
					// Send particles towards the black hole but with a
					// rotation to avoid going directly in.
					PVector corner1 = Tracker.normalizedToFloor(new PVector(1.0f, -1.0f));
					PVector corner2 = Tracker.normalizedToFloor(new PVector(-1.0f, 1.0f));
					PVector corner1Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner1))
							.normalize().mult(sourceVelocity).rotate(sourceAngle);
					PVector corner2Velocity = Tracker.getFloorCenter().sub(Tracker.normalizedToFloor(corner2))
							.normalize().mult(sourceVelocity).rotate(sourceAngle);

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
				particleFluff.velocity = Particle.genRandomVector(0.001f / 300, 0.0f, 0.0f);
				particleFluff.color = 0x99FFFFFF;
				particleFluff.scale = 0.5f;
				particleFluff.setLifespan(200);
				particleFluff.opacity = 0.3f;

				if (gameOver && intergameUpdateCounter > 300) {
					particleFluff.scale = 0.9f;
					particleFluff.opacity = 0.7f;

					// Use color + 1 so fluff doesn't get scored when the
					// game restarts.
					if (goal1.score >= winningScore) {
						particleFluff.color = goal1.color;
						universe.addParticle(particleFluff);
					}
					if (goal2.score >= winningScore) {
						particleFluff.color = goal2.color;
						universe.addParticle(particleFluff);
					}
				} else {
					universe.addParticle(particleFluff);
				}
			}
		}

		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			universe.attractor(pos.getOriginInMeters(), universe.settings.personForce);
		}

		// Scan particles for actions needed.
		for (int i = universe.particles.size() - 1; i >= 0; i--) {

			// Remove and score particles that have hit a goal.
			// TODO: Refactor for N goals.
			Particle particle = universe.particles.get(i);

			if (!gameOver) {
				goal1.enabled = true;
				goal2.enabled = true;

				if (goal1.goal(particle)) {
					goal1.scoreUp();
					universe.particles.remove(particle);
				} else if (goal2.goal(particle)) {
					goal2.scoreUp();
					universe.particles.remove(particle);
				}

				if (goal1.score >= winningScore) {
					gameOver = true;
					goal1.enabled = false;
					goal2.enabled = false;
					goal1.setSize(Tracker.getFloorDimensionMin() / 6);
					intergameUpdateCounter = updatesBetweenNextGame;
				}
				if (goal2.score >= winningScore) {
					gameOver = true;
					goal1.enabled = false;
					goal2.enabled = false;
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
		universe.attractor(Tracker.getFloorCenter(), 0.003f);
		
		// Clear the field before the next game starts.
		if (intergameUpdateCounter > 0 && intergameUpdateCounter < 300) {
			universe.attractor(Tracker.getFloorCenter(), 0.0001f * (300 - intergameUpdateCounter));
		}

		universe.update();
	}

	// Mix up the team color assignments from game to game using the teamAssignments map.
	private int teamColor(int id) {
		if (teamAssignment[id % teamAssignment.length] == true) {
			return color1;
		}
		return color2;
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		goal1.draw(g);
		goal2.draw(g);

		// Draw blackhole.
		blackhole.draw(g, 1.0f);

		// Draw people.
		g.noStroke();
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			g.fill(teamColor(id));
			g.ellipse(pos.getOriginInMeters().x, pos.getOriginInMeters().y, .3f, .3f);
		}
	}
}

