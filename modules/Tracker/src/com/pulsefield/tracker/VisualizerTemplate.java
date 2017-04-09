package com.pulsefield.tracker;
import processing.core.PApplet;
import processing.core.PGraphics;

// A Visualizer is responsible for all of the logic and display of a Pulsefield application
// Some visualizers also handle music/sound for the app
public class VisualizerTemplate extends Visualizer {

	VisualizerTemplate(PApplet parent) {
		// Constructor is called only once when main program starts up
		super();
	}

	@Override
	public void start() {
		super.start();
		// Other initialization called each time this app becomes active
	}
	
	@Override
	public void stop() {
		super.stop();
		// Cleanup called each time this app is deactivated
	}

	/* (non-Javadoc)
	 * @see com.pulsefield.tracker.Visualizer#update(processing.core.PApplet, com.pulsefield.tracker.People)
	 */
	@Override
	public void update(PApplet parent, People p) {
		// Update internal state using the positions/statistics of people in the Pulsefield
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		// Draw this app onto the pgraphics canvas which is mapped to the floor area of the pulsefield
		super.draw(t, g, p);
	}
}