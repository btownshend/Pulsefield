package com.pulsefield.tracker;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PGraphics;

public class VisualizerGrid extends VisualizerPS {
	Grid grid;
	
	VisualizerGrid(PApplet parent) {
		super(parent);
		final String songs[] = {"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR","PB"};
		grid=new Grid(songs);
	}
	
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		grid.start();
	}
	public void stop() {
		super.stop();
		grid.stop();
	}
	
	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
		grid.update(allpos);;
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		grid.draw(g, p);
		grid.drawTitle(g);
	}

	public void setnpeople(int n) {
		// Ignored for now
	}

	public void handleMessage(OscMessage theOscMessage) {
		if (!grid.handleMessage(theOscMessage))
			super.handleMessage(theOscMessage);
	}
}