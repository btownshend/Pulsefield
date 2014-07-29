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
	
	Leg(PVector position) { this.position=new PVector(position.x,position.y); }
	
	void move(PVector newpos, PVector newvel) {
		position=newpos;
		velocity=newvel;
//		PApplet.println("Leg moved to "+newpos+" with velocity "+newvel);
	}
	PVector getOriginInMeters() {
		return position;
	}
		
	PVector getVelocityInMeters() {
		return velocity;
	}
	float getDiameterInMeters() {
		return 0.2f; // TODO
	}
	float getMassInKg() {
		return 8.0f;  // TODO
	}
}
