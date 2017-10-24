package com.pulsefield.tracker;

import java.awt.Color;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

import java.awt.*;
import java.awt.image.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import javax.swing.*;

public class VisualizerWords extends VisualizerParticleSystem {
	String words = "Pulsefield";
	private float rainbowPositionDegrees = 0.0f;
	private float rainbowAdvance = 0.0f;
	float rainbowAdvanceIncrement = 0.1f; // rotation of the colors positions
											// per update.

	VisualizerWords(PApplet parent) {
		super(parent);
	}

	@Override
	void setUniverseDefaults() {
		ParticleSystemSettings pss = new ParticleSystemSettings();

		pss.maxParticles = 10000;
		pss.particleScale = 1.0f;
		pss.personForce = 0.002f;
		pss.particleRotation = 0.0f;
		pss.forceRotation = 0.0f;
		pss.particleMaxLife = 175;

		universe.settings = pss;
	}

	@Override
	public void start() {
		super.start();
	}

	// Create a particle specified by a distance and angle from a center point.
	private void spewParticle(ParticleSystem universe, int color, float rotation, float angle, float velocity,
			float distanceFromCenter, PVector center) {

		PVector origin = center.copy().add(new PVector(0, distanceFromCenter).rotate(rotation));
		// Particle p = new ImageParticle(origin, universe.settings, textures.get("blur.png"));
		Particle p = new TriangleParticle(origin, universe.settings);

		p.color = color;

		PVector velocityV = center.copy().sub(origin).normalize().setMag(velocity / 100).rotate(angle);
		p.velocity = velocityV;

		universe.addParticle(p);
	}

	@Override
	public void update(PApplet parent, People p) {
		int particlesPerUpdate = 360;
		rainbowAdvance = (rainbowAdvance + rainbowAdvanceIncrement) % 360;

		if (universe.particlesRemaining() >= 360) {
			for (int i = 0; i < 50; i++) {
				// Just some fluff particles for more visual interest.

				/*
				Particle particleFluff = new ImageParticle(
						Particle.genRandomVector(Tracker.getFloorDimensionMax() / 2, 0f, 0f), universe.settings,
						textures.get("blur.png"));
				*/

				Particle particleFluff = new TriangleParticle(Particle.genRandomVector(Tracker.getFloorDimensionMax() / 2, 0f, 0f), universe.settings);
				// particleFluff.velocity = Particle.genRandomVector(0.001f / 300, 0.0f, 0.0f);
				particleFluff.color = 0x99FFFFFF;
				particleFluff.scale = 1.0f;
				particleFluff.setLifespan(500);
				particleFluff.opacity = 0.7f;
				universe.addParticle(particleFluff);
			}
		}

		/*
		
		// Create some particles unless there are too many already.
		if (universe.particlesRemaining() >= 360) {
			for (int i = 0; i < particlesPerUpdate; i++) {
				rainbowPositionDegrees = (rainbowPositionDegrees + 1) % 360;
				int color = Color.HSBtoRGB((rainbowPositionDegrees + rainbowAdvance) / 360.0f, 1.0f, 1.0f);
		
				spewParticle(universe, color, (rainbowPositionDegrees / 360.0f) * 2 * (float) Math.PI, 0.0f, 0.0f,
						Tracker.getFloorDimensionMin() / 1.5f, Tracker.getFloorCenter());
			}
		}
		*/
		
		String words = "Pulse\nField\n<time>";

		if (words.contains("<time>")) {
			final long timestamp = new Date().getTime();
			final Calendar cal = Calendar.getInstance();
			cal.setTimeInMillis(timestamp);
			// and here's how to get the String representation
			final String timeString = new SimpleDateFormat("HH:mm:ss:SSS").format(cal.getTime());
		words = words.replace("<time>", timeString);
		}

		int[][] attArray = LettersToMatrix(words, 16);

		// double s = ((Math.sin(System.currentTimeMillis() / 10000)) + Math.PI) / Math.PI;
		// s = 1.0;
		


		float breathTime = 20000;
		float scale = (float) Math.sin((((System.currentTimeMillis() % ((int) breathTime)) / (breathTime / 2)) - 1) * Math.PI);

		float mult = Tracker.getFloorSize().x / (60.0f);
		for (int i = 0; i < attArray.length; i++) {
			for (int j = 0; j < attArray[i].length; j++) {
				if (attArray[i][j] != 0) {

					PVector subchar = new PVector(Tracker.getFloorCenter().x - (1 * (j * mult)) + Tracker.getFloorSize().x / 2.2f,
							Tracker.getFloorCenter().y + (1 * (i * mult)) - Tracker.getFloorSize().x / 2.2f);

					universe.attractor(subchar, (float) (0.000050f * scale * attArray[i][j]));

					// Apply drag to particle when it's near an attractor.
					for (Particle part : universe.particles) {

						float distance = PVector.sub(part.location, subchar).mag();

						if (distance < 0.1f) {
							if (scale < 0) {
								part.velocity = part.velocity.mult(1.1f);
							} else
								// Particles still move because acceleration builds!
								part.velocity = part.velocity.mult(0.00f);

						}

						/*
							float distance = 1 / PVector.sub(part.location, subchar).div(100.0f).magSq();
						
							float decel = 1 + 1 / (distance);
							// decel = 1.0f;
						
							part.velocity = part.velocity.div(decel);
						 */
					}
				}
			}
		}


		// applyPeopleGravity(p);

		// Generate particles around people.
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			Particle part = new TriangleParticle(pos.getOriginInMeters(), universe.settings);
			part.color = pos.getcolor();
			universe.addParticle(part);
		}

		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		drawPeople(g, p);
	}

	int[][] LettersToMatrix(String letters, int points) {
		Font font = new Font("SansSerif", Font.PLAIN, points);
		FontMetrics metrics = new JLabel().getFontMetrics(font);

		int height = 0;
		int width = 0;

		/*
		int width = metrics.stringWidth(letters);
		int height = metrics.getMaxAscent();
		*/

		// Calculate height and width.
		for (String line : letters.split("\n")) {
			height += metrics.getAscent();
			width = Math.max(width, metrics.stringWidth(line));
		}

		// Create buffered image to draw into.
		BufferedImage bi = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
		Graphics2D g2d = bi.createGraphics();
		g2d.setFont(font);
		g2d.setColor(Color.black);

		int y = metrics.getAscent();
		for (String line : letters.split("\n")) {
			g2d.drawString(line, 0, y);
			y += metrics.getAscent();
		}

		g2d.dispose();

		int[][] matrix = new int[height][width];

		for (int j = 0; j < height; j++) { // Scroll through rows
			for (int i = 0; i < width; i++) { // Scroll through columns
				// check for != 0 as black is -(2^24) and that's negative
				int v = bi.getRGB(i, j) != 0 ? 1 : 0;
				matrix[j][i] = v;
			}
		}

		return matrix;
	}
}
