import processing.core.PVector;

/**
 * 
 */

/**
 * @author bst
 *
 */
public class Leg {
	PVector position;
	PVector velocity;
	
	Leg() {; }
	
	void move(PVector newpos, PVector newvel) {
		position=newpos;
		velocity=newvel;
	}
	PVector getOriginInMeters() {
		return Tracker.unMapPosition(position);
	}
		
	PVector getVelocityInMeters() {
		return Tracker.unMapPosition(velocity);
	}
	float getDiameterInMeters() {
		return 0.2f; // TODO
	}
	float getMassInKg() {
		return 8.0f;  // TODO
	}
}
