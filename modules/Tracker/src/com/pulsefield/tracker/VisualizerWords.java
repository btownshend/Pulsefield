package com.pulsefield.tracker;

import java.awt.Color;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;

import java.awt.*;
import java.awt.image.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Random;

import javax.swing.*;

public class VisualizerWords extends VisualizerParticleSystem {
	String words = "Pulsefield";
	static Random rng = new Random();

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

	@Override
	public void update(PApplet parent, People p) {
		int particlesPerUpdate = 30;

		if (universe.particlesRemaining() >= particlesPerUpdate) {
			for (int i = 0; i < particlesPerUpdate; i++) {
				// Particle particleFluff = new TriangleParticle(Particle.genRandomVector(Tracker.getFloorDimensionMax()
				// / 2, 0f, 0f), universe.settings);
				Particle particleFluff = new ImageParticle(Particle.genRandomVector(Tracker.getFloorDimensionMax() / 2, 0f, 0f), universe.settings,
						textures.get("3x3-solid.png"));

				particleFluff.color = 0xFFFFFFFF;
				// Randomize scale from 1.2 to 2.2.
				particleFluff.scale = rng.nextFloat() + 1.2f;
				particleFluff.setLifespan(400);
				particleFluff.opacity = 0.6f;
				universe.addParticle(particleFluff);
			}
		}

		// TODO: Parameterize this .. perhaps move it into TouchOSC?
		String words = "     The\nPulsefield\n <time>";

		if (words.contains("<time>")) {
			final long timestamp = new Date().getTime();
			final Calendar cal = Calendar.getInstance();
			cal.setTimeInMillis(timestamp);
			final String timeString = new SimpleDateFormat("HH:mm:ss").format(cal.getTime());
			words = words.replace("<time>", timeString);
		}

		// The integer passed here represents the fineness of the font, a sort of pixels-per-inch.
		// A small number will make them blocky, made of few attractors. A large number will make
		// them smooth but will cause performance issues (many attractors).
		int[][] attArray = LettersToMatrix(words, 16);

		int height = attArray.length;
		int width = attArray[0].length;

		// Calculate constraining dimension for scale. Aim to make the rendering use the entire floor with
		// a little buffer.
		float xscale = Tracker.getFloorSize().x * 0.95f / width;
		float yscale = Tracker.getFloorSize().y * 0.95f / height;
		float xyscale = Math.min(xscale, yscale);

		// -1 multiplier due to field orientation.
		float ytop = Tracker.getFloorCenter().y - (-1 * xyscale * height / 2.0f);
		float xtop = Tracker.getFloorCenter().x - (-1 * xyscale * width / 2.0f);

		float direction = 1.0f;

		// Pulse-away the particles once every N seconds with varying strength (tuned for visual appeal).
		if ((System.currentTimeMillis() % 1500) < 400) {
			direction = -1 * (System.currentTimeMillis() % 60000) / 8000.0f;
		}

		for (int i = 0; i < attArray.length; i++) {
			for (int j = 0; j < attArray[i].length; j++) {
				if (attArray[i][j] != 0) {

					float xpos = xtop - (j * xyscale);
					float ypos = ytop - ((attArray.length - i) * xyscale);

					// Subcharacter attractor. Think of this as an e-ink particle.
					PVector subchar = new PVector(xpos, ypos);
					universe.attractor(subchar, (float) (direction * 0.000020f * attArray[i][j]));

					// Apply drag to particle when it's near an attractor to "stick" the particle.
					for (Particle part : universe.particles) {
						float distance = PVector.sub(part.location, subchar).mag();

						// Scale stickyness distance (which acts as a border around the font) with field
						// size (e.g. xyscale).
						if (distance < (0.6f * xyscale)) {
								part.velocity = part.velocity.mult(0.00f);
						}
					}
				}
			}
		}

		// Generate particles around people.
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			Particle part = new TriangleParticle(pos.getOriginInMeters(), universe.settings);
			part.scale = 1.2f;
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

	// Given text, return a 2d array plotting the text for later rendering with attractors.
	int[][] LettersToMatrix(String letters, int points) {
		Font font = new Font("SansSerif", Font.PLAIN, points);
		FontMetrics metrics = new JLabel().getFontMetrics(font);

		int height = 0;
		int width = 0;

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

		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				// Check for != 0 as black is -(2^24) and that's negative.
				int v = bi.getRGB(i, j) != 0 ? 1 : 0;
				matrix[j][i] = v;
			}
		}

		return matrix;
	}
}
