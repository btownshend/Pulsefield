package com.pulsefield.tracker;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

// Visualizer that draws a stickman for each person
class Stickman {
	// Joints are knee,hip,waist,neck,shoulder,elbow
	// Bodies are shank,thigh,lowerbody,upperbody,upp
	// Hierarchy  is based on hip
	// hip->thigh->shank
	// hip->waist->neck
	// neck->
	// Pivots are an adjustment relative to another pivot
	// Zero is defined so all segments are on top of each other, legs down, arms up
	// waist is an absolute angle 
	// Positive angles for left/right bodies rotate the point away from midline toward their side
	//  foot: relative to knee 
	//  knee: relative to hip 
	//  waist: relative to hip 
	//  neck: relative to waist 
	//  top: relative to neck
	//  elbow: relative to neck 
	//  hand: relative to elbow 
	// These are the angles of each pivot
	float LFOOT=-45, RFOOT=-45, LKNEE=45, RKNEE=45, WAIST=0, NECK=0, TOP=0, LELBOW=100, RELBOW=100, LHAND=30, RHAND=30;
	// Segments are part connecting two pivots
	// These are the lengths of each segment
	float SHANK, THIGH, LOWERBODY, UPPERBODY, HEAD, UPPERARM, LOWERARM;
	
	float height;
	PVector hipPosition;
	
	Stickman(float height){
		SHANK=height/5;THIGH=height/5;
		LOWERBODY=height/5;UPPERBODY=height/5;HEAD=height/5;
		UPPERARM=height/5;LOWERARM=height/5;
		hipPosition=new PVector(0,0);
		this.height=height;
	}
	// Update location of stickman 
	void updatePosition(Person p) {
		PVector left=p.legs[0].getOriginInMeters();
		PVector right=p.legs[1].getOriginInMeters();
		hipPosition=PVector.add(left,right).mult(0.5f);
		hipPosition.sub(0f,0.4f*height);
		// Use the x-coordinate to operate on the legs, y-coordinate to operate on the arms
		LKNEE=40*(float)Math.sin(left.x*2*Math.PI/1.0f)+40f;
		LFOOT=20*(float)Math.sin(left.x*2*Math.PI/1.2f)-20;
		RKNEE=40*(float)Math.sin(right.x*2*Math.PI/1.10f)+40f;
		RFOOT=20*(float)Math.sin(right.x*2*Math.PI/1.3f)-20;
		LELBOW=80*(float)Math.sin(left.y*2*Math.PI/1.0f)+90;
		LHAND=60*(float)Math.sin(left.y*2*Math.PI/1.2f);
		RELBOW=80*(float)Math.sin(right.y*2*Math.PI/1.1f)+90;
		RHAND=60*(float)Math.sin(right.y*2*Math.PI/1.3f);
		NECK=20*p.getVelocityInMeters().x;
		WAIST=10*p.getVelocityInMeters().y;
	}
	
	void destroy() {

	}
	
	float radians(float angle) { return PApplet.radians(angle); }
	
	void draw(PGraphics g) {
		g.pushMatrix();
		g.translate(hipPosition.x, hipPosition.y);
		g.rotate(radians(180));  // Flip
		g.rotate(radians(WAIST));
		g.pushMatrix(); {
			g.rotate(radians(LKNEE));
			g.line(0f, 0f, 0, -THIGH);
			g.translate(0f,-THIGH);
			g.rotate(radians(LFOOT));
			g.line(0f,0f, 0f, -SHANK);
			g.popMatrix();
		}
		g.pushMatrix(); {
			g.rotate(radians(-RKNEE));
			g.line(0f, 0f, 0, -THIGH);
			g.translate(0f,-THIGH);
			g.rotate(radians(-RFOOT));
			g.line(0f,0f, 0f, -SHANK);
			g.popMatrix();
		}
		//g.line(0f, 0f, rt.x, rt.y);
		//g.line(rt.x, rt.y,rt.x+(float)(-RSHANK*Math.sin(radians(RFOOT))), rt.y+(float)(-RSHANK*Math.cos(radians(RFOOT))));
		g.line(0f,0f,0f,LOWERBODY);
		g.translate(0f, LOWERBODY);
		g.rotate(radians(NECK));
		g.line(0f, 0f, 0f, UPPERBODY);
		g.translate(0f, UPPERBODY);
		g.pushMatrix(); {
			g.rotate(radians(LELBOW));
			g.line(0f, 0f, 0f, UPPERARM);
			g.translate(0f, UPPERARM);
			g.rotate(radians(LHAND));
			g.line(0f,0f,0f,LOWERARM);
			g.popMatrix();
		}
		g.pushMatrix(); {
			g.rotate(radians(-RELBOW));
			g.line(0f, 0f, 0f, UPPERARM);
			g.translate(0f, UPPERARM);
			g.rotate(radians(-RHAND));
			g.line(0f,0f,0f,LOWERARM);
			g.popMatrix();
		}
		g.rotate(radians(TOP));
		g.translate(0f, HEAD/2);
		g.ellipseMode(PConstants.CENTER);
		g.arc(0f, 0f, HEAD, HEAD, 0, radians(360));
		g.popMatrix();
	}
	
	static void updateAll(Effects effects) {

	}

}

public class VisualizerStickman extends Visualizer {
	HashMap<Integer, Stickman> sticks;
	Effects effects;


	VisualizerStickman(PApplet parent, Synth synth) {
		super();
		sticks = new HashMap<Integer, Stickman>();
		effects=new Effects(synth,123);
		effects.add("COLLIDE",52,55);
		effects.add("SPLIT",40,42);
	}
	
	public void update(PApplet parent, People allpos) {		
		// Update internal state of the Marbles
		for (int id: allpos.pmap.keySet()) {
			if (!sticks.containsKey(id))
				sticks.put(id,new Stickman(1.8f));
			sticks.get(id).updatePosition(allpos.get(id));
			//PApplet.println("Marble "+id+" moved to "+currentpos.toString());
		}
		// Remove Marbles for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = sticks.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				sticks.get(id).destroy();
				iter.remove();
			}
		}
		Stickman.updateAll(effects);
	}
	
	/**
	 * Draw a stickman using context current stroke settings
	 * @param g - graphics context
	 * @param left - left leg
	 * @param right - right leg
	 * @param direction - direction that stickman stands in degrees
	 * @param height - height in meters to top of head
	 */
	
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		
		g.ellipseMode(PConstants.CENTER);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.noFill();
			g.stroke(c,255);
			sticks.get(ps.id).draw(g);
		}
	}
}

