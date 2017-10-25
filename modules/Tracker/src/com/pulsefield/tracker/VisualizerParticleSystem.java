package com.pulsefield.tracker;

import java.io.File;
import java.util.HashMap;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;

class VisualizerParticleSystem extends Visualizer {
	ParticleSystem universe;
	String oscName = "particlefield";

	HashMap<String, PImage> textures;

	VisualizerParticleSystem(PApplet parent) {
		loadTextures(parent);
	}

	void loadTextures(PApplet parent) {
		textures = new HashMap<String, PImage>();

		// Load all texture images into a hashmap for name access or iteration.
		File directory = new File("data/particleimages/");
		File[] files = directory.listFiles();
		if (files == null || files.length <= 0) {
			PApplet.println("No image files to load");
		} else {
			for (File file : files) {
				if (file.isFile()) {
					try {
						textures.put(file.getName(), parent.loadImage(file.getAbsolutePath()));
						logger.info("Loaded particle texture filename " + file.getAbsolutePath());
					} catch (Exception e) {
						PApplet.println("Failed to load image " + file.getAbsolutePath());
					}
				}
			}
		}
	}

	@Override
	public void start() {
		super.start();
		universe = new ParticleSystem();
		
		setUniverseDefaults();
		setTO();
	}
	
	@Override
	public void stop() {
		universe = null;
		super.stop();
	}
	
	// setUniverseDefaults is a method for subclasses to specify their defaults.
	void setUniverseDefaults() {
	}

	@Override
	public void update(PApplet parent, People p) {
		universe.update();
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		
		if (! universe.dead()) {
			universe.draw(g);
		}
	}

	// Apply gravity for persons scaling the effect based on their leg
	// separation.
	void applyPeopleGravityLegScaled(People p) {
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			universe.attractor(pos.getOriginInMeters(),
					(float) (universe.settings.personForce * (1.2 - pos.getLegSeparationInMeters())));
		}
	}
	
	// Apply gravity for persons scaling the effect based on their leg
	// separation.
	void applyPeopleGravity(People p) {
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			universe.attractor(pos.getOriginInMeters(),universe.settings.personForce);
		}
	}

	void drawPeople(PGraphics g, People p) {
		for (int id : p.pmap.keySet()) {
			String label = "+";

			Person pos = p.pmap.get(id);
			g.fill(0xEEFFFFFF);
			g.textAlign(PConstants.CENTER, PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, label, pos.getOriginInMeters().x, pos.getOriginInMeters().y);
		}
	}

	@Override
	public void handleMessage(OscMessage msg) {
		if (universe == null)
			universe = new ParticleSystem();

		logger.fine("Particle message: " + msg.toString());

		String pattern = msg.addrPattern();
		String components[] = pattern.split("/");
		PApplet.println("ParticleField OSC Message (" + components.length + " len) " + msg.toString() + " : "
				+ msg.get(0).floatValue());

		if (components.length < 3 || !components[2].equals(oscName))
			logger.warning("VisualizerParticleSystem: Expected /video/" + oscName + " messages, got " + msg.toString());
		else if (components.length == 4 && components[3].equals("maxparticles")) {
			universe.settings.maxParticles = (long) Math.pow(2, msg.get(0).floatValue());
		} else if (components.length == 4 && components[3].equals("particledispersion")) {
			universe.settings.particleRandomDriftAccel = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("forcerotation")) {
			universe.settings.forceRotation = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlerotation")) {
			universe.settings.particleRotation = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlemaxlife")) {
			universe.settings.particleMaxLife = (int) msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particleopacity")) {
			universe.settings.startOpacity = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("persongravity")) {
			universe.settings.personForce = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlescale")) {
			universe.settings.particleScale = msg.get(0).floatValue();
		} else if (components.length == 6 && components[3].equals("blendmode")) {
			handleBlendSettingMessage(msg);
		} else if (components.length == 4 && components[3].equals("tilt")) {
			handleTilt(msg);
		} else
			logger.warning("Unknown " + oscName + " Message: " + msg.toString());

		setTO();
	}

	private void handleTilt(OscMessage msg) {
		msg.addrPattern().split("/");

		universe.settings.tiltx = msg.get(0).floatValue() * 0.0001f;
		universe.settings.tilty = msg.get(1).floatValue() * 0.0001f;
	}

	private void handleBlendSettingMessage(OscMessage msg) {
		String components[] = msg.addrPattern().split("/");

		if (components.length < 5) {
			logger.warning("handleBlendSettingMessage: Expected more components in " + msg.toString());
		} else {
			// If it's an "off" (0.0) message, ignore it.
			if (msg.get(0).floatValue() == 0.0) {
				return;
			}
			String clicked = components[4].toString() + "/" + components[5].toString();

			switch (clicked) {
			case "2/1":
				universe.settings.blendMode = -1; // Custom.
				break;
			case "2/2":
				universe.settings.blendMode = PConstants.ADD;
				break;
			case "2/3":
				universe.settings.blendMode = PConstants.SUBTRACT;
				break;
			case "2/4":
				universe.settings.blendMode = PConstants.SCREEN;
				break;
			case "1/1":
				universe.settings.blendMode = PConstants.LIGHTEST;
				break;
			case "1/2":
				universe.settings.blendMode = PConstants.BLEND;
				break;
			case "1/3":
				universe.settings.blendMode = PConstants.OVERLAY;
				break;
			case "1/4":
				universe.settings.blendMode = PConstants.MULTIPLY;
				break;
			}
		}
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to = TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/" + oscName + "/" + name);
		set.add(value);
		to.sendMessage(set);
		set = new OscMessage("/video/" + oscName + "/" + name + "/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}

	public void setTO() {
		setTOValue("maxparticles", Math.log(universe.settings.maxParticles) / Math.log(2), "%.4f");
		setTOValue("particleaccel", universe.settings.particleRandomDriftAccel, "%.4f");
		setTOValue("forcerotation", universe.settings.forceRotation, "%.4f");
		setTOValue("particlerotation", universe.settings.particleRotation, "%.4f");
		setTOValue("particlemaxlife", universe.settings.particleMaxLife, "%.4f");
		setTOValue("particleopacity", universe.settings.startOpacity, "%.4f");
		setTOValue("persongravity", universe.settings.personForce, "%.4f");
		setTOValue("particlescale", universe.settings.particleScale, "%.4f");

		setTiltTO();
	}

	public void setTiltTO() {
		TouchOSC to = TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/" + oscName + "/tilt");
		set.add("set");
		set.add(universe.settings.tiltx);
		set.add(universe.settings.tilty);
		to.sendMessage(set);
	}
}