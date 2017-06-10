package com.pulsefield.tracker;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PShape;
import processing.core.PVector;



class LinkedPerson {
	Person match;
	Person me;
	float timetogether = 0;
	final float TIMETOSCORE = 20.f;
	final float TIMEFOREXPLOSION = 10.f;
	final float radius=(float)(0.69/2/Math.PI);
	final float arrowlen=0.2f;
	final float JOINTHRESH=2 * radius + arrowlen;
	final float STARTTHRESH=JOINTHRESH + 0.1f;
	final float EXPLOSIONSIZE = 0.5f;
	boolean exploding = false;
	float timeexploding = 0.f;
	Integer score = 0;
	
	public LinkedPerson(Person p) {
		this.me = p;
	}
	
	void givePartner(HashMap<Integer, LinkedPerson> links) {
		if (this.match==null) {
			List<Map.Entry<Integer,LinkedPerson>> list = new ArrayList<Map.Entry<Integer,LinkedPerson>>(links.entrySet());

			// each time you want a different order.
			Collections.shuffle(list);
			
			for (Map.Entry<Integer, LinkedPerson> entry: list) {
				LinkedPerson other = entry.getValue();
				Integer otherId = entry.getKey();
				if(otherId != me.id && (other.match == null || other.match.id != me.id)) { 
					match = links.get(otherId).me;
					if (isCloseToPartner(STARTTHRESH)) {
						this.match = null;
					}
					else {
						PApplet.println("Linked ID "+ me.id + " to ID " + otherId);
						break;
					}
				}
			}
		}
	}
	
	boolean isCloseToPartner(float thresh) {
		if (this.match == null) {
			return false;
		}
		if (PVector.dist(this.match.getOriginInMeters(), this.me.getOriginInMeters())<thresh) {
			return true;
		}
		return false;
	}
	
	
	void update(HashMap<Integer, LinkedPerson> links) {

		
		if (exploding) {
			timeexploding += 1;
			if (timeexploding > TIMEFOREXPLOSION) {
				timeexploding = 0;
				exploding = false;
			}
		}
		else {
			givePartner(links);
		}
		
		if (isCloseToPartner(JOINTHRESH)) {

			timetogether += 1;
			if (this.timetogether > TIMETOSCORE) {
				this.score += 1;
				this.match = null;
				this.timetogether = 0;
				exploding = true;
			}
		}
		else {
			timetogether = 0;
		}
	}
	
	float radians(float angle) { return PApplet.radians(angle); }
	
	void draw(PGraphics g) {
		PVector mypos = this.me.getOriginInMeters();
		if (exploding) {
			g.stroke(255, 0, 0, 255);
			
			for (int angle=0; angle < 360; angle+=20) {
				float rad = radians(1.f * angle);
				PVector direction = PVector.fromAngle(rad);
				PVector diff = PVector.mult(direction,  radius + (timeexploding / TIMEFOREXPLOSION) * EXPLOSIONSIZE);
				PVector diff2 = PVector.mult(direction, radius + ((timeexploding + 0.5f) / TIMEFOREXPLOSION) * EXPLOSIONSIZE);
				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff2.x, mypos.y + diff2.y);
			}
		}
		g.ellipse(mypos.x,mypos.y, radius*2, radius*2);
		g.pushStyle(); {
			g.stroke(255, 0, 0, 255);
			g.arc(mypos.x, mypos.y, radius*2, radius*2, radians(-90), radians(-90 + 360.f * (timetogether / TIMETOSCORE)));
			g.popStyle();
		}
		if (this.match != null) {
			PVector otherpos = this.match.getOriginInMeters();	
			PVector uv = PVector.sub(otherpos,mypos);
			float distance = uv.mag();
			uv.normalize();
		
			
			PVector diff = PVector.mult(uv, Math.min(arrowlen + radius, distance - radius));
			PVector diff2 = PVector.mult(uv, radius);
			if (distance >= 2 * radius) {
				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff2.x, mypos.y + diff2.y);
			}
		}
		
		Visualizer.drawText(g, 0.10f, "" + this.score, mypos.x + radius / 2, mypos.y + radius / 3);
	}
}

// A Visualizer is responsible for all of the logic and display of a Pulsefield application
// Some visualizers also handle music/sound for the app
public class VisualizerHunter extends Visualizer {
	HashMap<Integer, LinkedPerson> links = new HashMap<Integer, LinkedPerson>();
	PShape ballShape;
	
	VisualizerHunter(PApplet parent) {
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
	public void update(PApplet parent, People allpos) {		
		for (int id: allpos.pmap.keySet()) {
		if (!links.containsKey(id))
			links.put(id,new LinkedPerson(allpos.pmap.get(id)));
		//PApplet.println("Marble "+id+" moved to "+currentpos.toString());
		}
		// Remove Marbles for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = links.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				iter.remove();
			}
		}
		
		List<Map.Entry<Integer,LinkedPerson>> list = new ArrayList<Map.Entry<Integer,LinkedPerson>>(links.entrySet());

		// each time you want a different order.
		Collections.shuffle(list);
		
		for (Map.Entry<Integer, LinkedPerson> entry: list) {
			entry.getValue().update(links);
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		// Draw this app onto the pgraphics canvas which is mapped to the floor area of the pulsefield
		super.draw(t, g, p);
		
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.noFill();
			g.stroke(c,255);
			links.get(ps.id).draw(g);
		}
		g.shapeMode(PConstants.CENTER);
		
	}
}