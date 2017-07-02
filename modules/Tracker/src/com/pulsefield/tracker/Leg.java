package com.pulsefield.tracker;
import java.util.logging.Logger;

import processing.core.PVector;

/**
 * 
 */

/**
 * @author bst
 *
 */
public class Leg {
	private PVector position;
	private PVector velocity;
    private final static Logger logger = Logger.getLogger(Leg.class.getName());

	Leg(PVector position) { this.position=new PVector(position.x,position.y); }
	
	void move(PVector newpos, PVector newvel) {
		position=new PVector(newpos.x,newpos.y);
		velocity=new PVector(newvel.x,newvel.y);
//		logger.fine("Leg moved to "+newpos+" with velocity "+newvel);
	}
	PVector getOriginInMeters() {
		return position;
	}
	// Convert to normalized position - in range [-1,1] for extent of pulsefield
	PVector getNormalizedPosition() {
		return Tracker.floorToNormalized(position);
	}
	PVector getVelocityInMeters() {
		return velocity;
	}
	PVector getNormalizedVelocity() {
		return Tracker.mapVelocity(velocity);
	}
	float getDiameterInMeters() {
		return 0.2f; // TODO
	}
	float getMassInKg() {
		return 8.0f;  // TODO
	}
}
