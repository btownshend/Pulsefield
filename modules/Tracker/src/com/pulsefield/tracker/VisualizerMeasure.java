// This simple Visualizer draws lines between each Person and labels their distance.

package com.pulsefield.tracker;

import java.util.HashMap;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

class Measure {
	PVector position;
    protected final static Logger logger = Logger.getLogger(Measure.class.getName());

	Measure(PVector pos) { 
		position=pos;
	}
	
	void draw(PGraphics g, People p) {
		g.imageMode(PConstants.CENTER);
		g.translate(position.x, position.y);
		g.colorMode(PGraphics.RGB);
		
		// Create a copy of the people list; we'll walk the full list and for each element
		// we'll walk the copy; removing the covered person from the copy in each 
		// pass to avoid the duplicate (reverse) pass.
		HashMap<Integer, Person> peopleCopy = new HashMap<Integer,Person>(p.pmap);
		
		for (Person ps: p.pmap.values()) {
			for (Person ps2: peopleCopy.values()) {
				// Don't interact with one's self.
				if (ps.id == ps2.id) continue;
						
				int c1=ps.getcolor();
				int c2=ps2.getcolor();
				int newcolor=(((c1>>>1)&0x7f7f7f) + ((c2>>>1)&0x7f7f7f)) | 0xff000000; // Combined color
				//logger.info("c1="+PApplet.hex(c1)+", c2="+PApplet.hex(c2)+", newcolor="+PApplet.hex(newcolor));
				g.stroke(newcolor);
				// Draw the line between the two persons.
				g.line(ps.getOriginInMeters().x,  ps.getOriginInMeters().y,
						ps2.getOriginInMeters().x,  ps2.getOriginInMeters().y);
				
				float distance = PVector.dist(ps.getOriginInMeters(), ps2.getOriginInMeters());
				
				// Label distance on the line segment if it's long enough.
				// TODO: Instead of suppressing, have text size scale?
				if (distance > 0.25f) { 

					float midx = ((ps.getOriginInMeters().x + ps2.getOriginInMeters().x) / 2);
					float midy = ((ps.getOriginInMeters().y + ps2.getOriginInMeters().y) / 2);

					g.pushMatrix();
					
					// Move to the location we want the text, then rotate to match the line angle.
					g.translate(midx, midy);
					
					// Text is backwards without this scaling step.
					g.scale(1, -1);
					
					// Bit of trig to determine the angle.
					double xlen = (ps.getOriginInMeters().x - ps2.getOriginInMeters().x);
					double ylen = (ps.getOriginInMeters().y - ps2.getOriginInMeters().y);
					double angle = Math.atan(xlen / ylen);

					// Rotate the canvas the angle amount plus 90 degrees (so
					// the text is perpendicular with the line drawn).
					g.rotate((float) ((angle) + Math.PI / 2));

					g.textAlign(PGraphics.CENTER, PGraphics.CENTER);
					g.textSize((float) 0.15);

					// Create a slight black border around the text by drawing
					// it black with an offset.
					g.fill(0xffffffff); // Full opacity; black.
					float gap = 0.01f;
					String tFormat = "%.02f";
					g.text(String.format(tFormat, distance) + "m", gap, gap);
					g.text(String.format(tFormat, distance) + "m", -gap, gap);
					g.text(String.format(tFormat, distance) + "m", gap, -gap);
					g.text(String.format(tFormat, distance) + "m", -gap, -gap);

					g.fill(0xffee2222); // Full opacity; red.
					g.text(String.format(tFormat, distance) + "m", 0, 0);

					g.popMatrix();
				}
			}
			
			// Remove the completed point so we don't revisit it's inverse segment.
			// TODO: Double check this is working as expected.
			peopleCopy.remove(ps);
		}

	}
	
	void update(People p) {
	}
	
}

public class VisualizerMeasure extends VisualizerGrid {
	Measure measure;

	VisualizerMeasure(PApplet parent) {
		super(parent);
		drawGrid=false;
		measure=new Measure(new PVector(0f,0f));
	}
	
	@Override
	public void start() {
		super.start();
		// trackSet taken care of in Grid.start()
	}
	
	public void update(PApplet parent, People p) {
		super.update(parent,p);
		measure.update(p);
	}
	
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		// g.shapeMode(PApplet.CENTER);
		measure.draw(g, p);
	}
}
