package com.pulsefield.tracker;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

/*** Multiplayer tag where everyone is trying to get one other person at a time. ***/

class Hunter {
	Hunter target;  // Targeted person.
	Person me; // My person.
	float timetogether = 0; // Consecutive time I have spent next to my target.	(sec)
	float timeexploding = 0.f; // Time I have been exploding for. (sec)
	boolean exploding = false; // If I am exploding (True if I just tagged someone).
	Integer score = 0; // Number of people I have tagged.
	boolean deleted = false; // Whether I still exist.


	/*** General params. ***/
	// Radius of a player.
	final float RADIUS=(float)(0.69/2/Math.PI)*2;
	// Length of arrow pointing to current target.
	final float ARROWLEN=0.8f;
	final float ARROWHEADLEN=0.1f;
	
	/*** Tag params. ***/
	// Number of seconds that a person needs to be next to another to tag them.
	final float TIMETOTAG=1.0f;
	// Distance threshold at which a player is considered to be tagging another. (meters)
	final float TAGTHRESH=0.6f;
	// Distance threshold a player needs to be from another to have them be started as a target. (meters)
	final float STARTTHRESH=TAGTHRESH + 0.1f;
	
	
	/*** Explosion params. ***/
	// Number of seconds that an explosion will take place for upon tagging.
	final float TIMEFOREXPLOSION = 2f;
	// Radius of explosion upon tagging (in meters)
	final float EXPLOSIONSIZE = 3f;

    private final static Logger logger = Logger.getLogger(Hunter.class.getName());

	
	public Hunter(Person p) {
		this.me = p;
	}
	
	/***
	 * Find a target for this hunter.
	 * 
	 * @param players Potential targets.
	 */
	void giveTarget(HashMap<Integer, Hunter> players) {
		if (this.target==null) {
			List<Map.Entry<Integer,Hunter>> list = new ArrayList<Map.Entry<Integer,Hunter>>(players.entrySet());
			Collections.shuffle(list);
			
			for (Map.Entry<Integer, Hunter> entry: list) {
				Hunter other = entry.getValue();
				Integer otherId = entry.getKey();
				
				// Don't want to pick myself or someone who is targeting me or someone who is too close.
				if(otherId != me.id && (other.target == null || other.target.me.id != me.id) && !isCloseTo(STARTTHRESH, other)) { 
					target = players.get(otherId);
					logger.fine("Linked ID "+ me.id + " to ID " + otherId);
					break;
				}
			}
		}
	}
	
	/***
	 * Check if within a certain distance of other person.
	 * 
	 * @param thresh distance to use.
	 * @param other other person to consider
	 * @return boolean indicating if we are within the provided distance of said person.
	 */
	boolean isCloseTo(float thresh, Hunter other) {
		if (other == null) {
			return false;
		}
		if (PVector.dist(other.me.getOriginInMeters(), this.me.getOriginInMeters())<thresh) {
			return true;
		}
		return false;
	}
	
	/***
	 * Mark ourselves as deleted if we are gone.
	 */
	void destroy() {
		logger.fine("Destroying hunter id "+me.id);
		deleted = true;
	}
	
	
	/***
	 * Update this hunter as necessary.
	 * 
	 * @param players Other players playing.
	 */
	void update(HashMap<Integer, Hunter> players, Effects effects) {
		// If we are exploding that is all we do.
		if (exploding) {
			timeexploding += 1.0f/Tracker.theTracker.frameRate;
			if (timeexploding > TIMEFOREXPLOSION) {
				timeexploding = 0;
				exploding = false;
			}
		}
		else {
			// If target left the field, we reset our target.
			if (this.target != null && this.target.deleted) {
				logger.fine("Target "+this.target.me.id+" was deleted - clearing");
				this.target = null;
			}
			// If not target, we try and get one.
			if (this.target == null) {
				giveTarget(players);
			}
			// If we are close we start tagging them.
			if (isCloseTo(TAGTHRESH, this.target)) {
				timetogether += 1f/Tracker.theTracker.frameRate;
				// If we are close to someone for long enough we explode!
				if (this.timetogether > TIMETOTAG) {
					this.score += 1;
					this.target = null;
					this.timetogether = 0;
					exploding = true;
					logger.fine("Exploding "+me.id);
					effects.play("SHOTGUN", 127, 1000);
				}
			}
			// If not, we reset our tagging progress.
			else {
				timetogether = 0;
			}
		}
	}
	
	/***
	 * Convert degrees to radians.
	 * 
	 * @param angle value in degrees.
	 * @return angle in radians.
	 */
	float radians(float angle) { return PApplet.radians(angle); }
	
	/***
	 * Draw this hunter.
	 * 
	 * @param g
	 */
	void draw(PGraphics g) {
		PVector mypos = this.me.getOriginInMeters();
		
		// Draw explosion if needed.
		if (exploding) {
			g.pushStyle();
			g.strokeWeight(0.05f);
			g.stroke(255, 0, 0, 255);
			
			for (int i=0; i<50;i++) {
				float angle=(float)(Math.random()*Math.PI*2);
				float dist=(float)(RADIUS + Math.random()*(timeexploding / TIMEFOREXPLOSION) * EXPLOSIONSIZE);
				float dist2=dist+0.2f;
				PVector direction = PVector.fromAngle(angle);
				PVector diff = PVector.mult(direction,  dist);
				PVector diff2 = PVector.mult(direction, dist2);
				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff2.x, mypos.y + diff2.y);
			}
			g.popStyle();
		}
		
		// Our own circle.
		g.ellipse(mypos.x,mypos.y, RADIUS*2, RADIUS*2);
		
		// Our tagging progress.
		g.pushStyle(); {
			g.stroke(255, 0, 0, 255);
			g.strokeWeight(0.1f);
			g.arc(mypos.x, mypos.y, RADIUS*2, RADIUS*2, radians(-90), radians(-90 + 360.f * (timetogether / TIMETOTAG)));
			g.popStyle();
		}
		
		// Arrow pointing at our target, if we have one.
		if (this.target != null) {
			PVector otherpos = this.target.me.getOriginInMeters();	
			PVector uv = PVector.sub(otherpos,mypos);
			float distance = uv.mag();
			uv.normalize();
			if (distance >= 2 * RADIUS) {
				PVector diff = PVector.mult(uv, Math.min(ARROWLEN + RADIUS, distance - RADIUS));
				PVector diff2 = PVector.mult(uv, RADIUS);

				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff2.x, mypos.y + diff2.y);
				PVector arrow=PVector.mult(uv,ARROWHEADLEN);
				arrow.rotate((float)Math.PI*135/180);
				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff.x+ arrow.x, mypos.y + diff.y + arrow.y);
				arrow.rotate((float)Math.PI/2);
				g.line(mypos.x + diff.x, mypos.y + diff.y, mypos.x + diff.x+ arrow.x, mypos.y + diff.y + arrow.y);
			}
		}
		
		// Our own score.
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		Visualizer.drawText(g, 0.30f, "" + this.score, mypos.x, mypos.y -0.05f);
	}
}

public class VisualizerHunter extends Visualizer {
	// List of current players.
	HashMap<Integer, Hunter> players = new HashMap<Integer, Hunter>();
	Effects effects;
	
	VisualizerHunter(PApplet parent, Synth synth) {
		super();
		effects=Effects.defaultEffects;
	}

	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Hunter");
	}

	@Override
	public void update(PApplet parent, People allpos) {		
		
		// Add new players if needed.
		for (int id: allpos.pmap.keySet()) {
			if (!players.containsKey(id)) {
				logger.fine("Adding ID "+id);
				players.put(id,new Hunter(allpos.pmap.get(id)));
			}
		}
		
		// Remove players that left if needed.
		for (Iterator<Integer> iter = players.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				logger.fine("Removing ID "+id);
				players.get(id).destroy();
				iter.remove();
			}
		}
		
		// Update the hunters in a random order.
		List<Map.Entry<Integer,Hunter>> list = new ArrayList<Map.Entry<Integer,Hunter>>(players.entrySet());
		Collections.shuffle(list);
		for (Map.Entry<Integer, Hunter> entry: list) {
			entry.getValue().update(players, effects);
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.noFill();
			g.stroke(c,255);
			if (players.get(ps.id)!=null)
				players.get(ps.id).draw(g);
		}
		g.shapeMode(PConstants.CENTER);
	}
}
