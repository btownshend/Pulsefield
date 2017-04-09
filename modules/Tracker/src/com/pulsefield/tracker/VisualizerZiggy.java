package com.pulsefield.tracker;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;

public class VisualizerZiggy extends VisualizerWhack {
	PImage playadust;
	
	VisualizerZiggy(PApplet parent, String dataDir, String trackSet, String whackEffect) {
		super(parent, dataDir, trackSet, whackEffect);
		playadust = parent.loadImage("bowie/playadust.jpg");
	}

	@Override
	public void start() {
		super.start();
		// Other initialization when this app becomes active
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
		super.update(parent,p);
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
//		g.tint(255,255);
//		g.fill(200,200,10);
//		g.textAlign(PConstants.CENTER,PConstants.CENTER);
//		drawText(g, 0.4f, "Ziggy Playadust & the Seed Bugs", (Tracker.minx+Tracker.maxx)/2, (Tracker.miny+Tracker.maxy)/2);
		
		g.tint(200);
		g.imageMode(PConstants.CENTER);
		float glength=Tracker.getFloorSize().x; // Length of guitar in meters
		//float gheight=glength*guitar.height/guitar.width*2;
		float gheight=Tracker.getFloorSize().y*playadust.height/playadust.width;
		g.pushMatrix();
		g.translate(Tracker.getFloorCenter().x, Tracker.getFloorCenter().y);
		g.scale(-1,1);
		//g.image(playadust, 0,0, glength, gheight);
		g.popMatrix();
		// Add drawing code here
	}
	
}
