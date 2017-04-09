package com.pulsefield.tracker;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

// Visualizer that just displays a dot for each person

public class VisualizerDot extends Visualizer {
	
	VisualizerDot(PApplet parent) {
		super();

	}
	
	public void update(PApplet parent, People p) {
		;
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		g.ellipseMode(PConstants.CENTER);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			for (int i=0;i<ps.legs.length;i++) {
				Leg leg=ps.legs[i];
				PVector pos=leg.getOriginInMeters();
				g.ellipse(pos.x, pos.y, leg.getDiameterInMeters(), leg.getDiameterInMeters());
			}
		}
	}
}

