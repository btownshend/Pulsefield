package com.pulsefield.tracker;

import java.awt.Color;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;
import sun.swing.PrintColorUIResource;
import processing.core.PImage;

class VisualizerParticleSystem extends Visualizer {
	ParticleSystem universe;
	float personGravity = 0.005f;
	float particleScale = 1.0f;
	String oscName = "particlefield";
	
	VisualizerParticleSystem(PApplet parent) {
	}
	
	@Override
	public void start() {
		super.start();
		universe = new ParticleSystem();
		
		PApplet.println("My VPS universe is " + universe);
		Laser.getInstance().setFlag("body", 0.0f);
		setTO();
	}

	
	@Override
	public void update(PApplet parent, People p) {
		universe.update();
	}
	
	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		universe.draw(g, particleScale);
	}
	
	void applyPeopleGravity(People p) {
		for (int id : p.pmap.keySet()) {
			Person pos = p.pmap.get(id);
			universe.attractor(pos.getOriginInMeters(), personGravity * (1 - pos.getLegSeparationInMeters()));
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
	
	public void handleMessage(OscMessage msg) {
		if (universe == null) universe = new ParticleSystem();
		
		logger.fine("Particle message: "+msg.toString());
		
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		PApplet.println("ParticleField OSC Message ("+ components.length + " len) "+ msg.toString() + " : " + msg.get(0).floatValue());
		
		if (components.length<3 || !components[2].equals(oscName)) 
			logger.warning("VisualizerParticleSystem: Expected /video/" + oscName + " messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("maxparticles")) {
			universe.maxParticles= (long) Math.pow(2, msg.get(0).floatValue());
		} else if (components.length==4 && components[3].equals("particleaccel")) {
			universe.particleRandomDriftAccel = msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("forcerotation")) {
			universe.forceRotation = msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("particlerotation")) {
			universe.particleRotation = msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("particlemaxlife")) {
			universe.particleMaxLife = (int) msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("background")) {
			universe.backgroundBrightness = msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("particleopacity")) {
			universe.startOpacity = msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("persongravity")) {
			personGravity=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("particlescale")) {
			particleScale = msg.get(0).floatValue();
		} else if (components.length==6 && components[3].equals("blendmode")) {
			handleBlendSettingMessage(msg);
		} else if (components.length==4 && components[3].equals("tilt")) {
			handleTilt(msg);
		} else 
			logger.warning("Unknown " + oscName + " Message: "+msg.toString());
				
		setTO();
	}
	
	
	private void handleTilt(OscMessage msg) {
		String components[]=msg.addrPattern().split("/");

		universe.tiltx = msg.get(0).floatValue() * 0.0001f;
		universe.tilty = msg.get(1).floatValue() * 0.0001f;
	}
	
	private void handleBlendSettingMessage(OscMessage msg) {
		String components[]=msg.addrPattern().split("/");

		if (components.length<5) {
			logger.warning("handleBlendSettingMessage: Expected more components in "+msg.toString());
		} else {
			// If it's an "off" (0.0) message, ignore it.
			if (msg.get(0).floatValue() == 0.0) {
				return;
			}
			String clicked = components[4].toString() + "/" + components[5].toString();

			switch (clicked) { 
			case "2/1": universe.blendMode = PGraphics.BLEND;
			break;
			case "2/2": universe.blendMode = PGraphics.ADD;
			break;
			case "2/3": universe.blendMode = PGraphics.SUBTRACT;
			break;
			case "2/4": universe.blendMode = PGraphics.SCREEN;
			break;
			case "1/1": universe.blendMode = PGraphics.LIGHTEST;
			break;
			case "1/2": universe.blendMode = PGraphics.DARKEST;
			break;
			case "1/3": universe.blendMode = PGraphics.OVERLAY;
			break;
			case "1/4": universe.blendMode = PGraphics.MULTIPLY;
			break;
			}
		}
	}
	
	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/" + oscName + "/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/video/" + oscName + "/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}
	
	public void setTO() {
		setTOValue("maxparticles",Math.log(universe.maxParticles)/Math.log(2) ,"%.4f");
		setTOValue("particleaccel",universe.particleRandomDriftAccel,"%.4f");
		setTOValue("forcerotation",universe.forceRotation,"%.4f");
		setTOValue("particlerotation",universe.particleRotation,"%.4f");
		setTOValue("particlemaxlife",universe.particleMaxLife, "%.4f");
		setTOValue("particleopacity",universe.startOpacity,"%.4f");
		setTOValue("background",universe.backgroundBrightness,"%.4f");
		setTOValue("persongravity",personGravity,"%.4f");
		setTOValue("particlescale",particleScale,"%.4f");
	}
}