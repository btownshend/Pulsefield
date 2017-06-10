package com.pulsefield.tracker;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

// Visualizer that draws a stickman for each person
// Would be nice to:
// - use some inverse kinematics (such as https://github.com/EGjoni/Everything-WIll-Be-IK )
// - let people hold hands
// - faces?

class HandHold {
	Stickman s1, s2;
	boolean isLeft1, isLeft2;
	HandHold(Stickman _s1, Stickman _s2, boolean _isLeft1, boolean _isLeft2) {
		s1=_s1; isLeft1=_isLeft1;
		s2=_s2; isLeft2=_isLeft2;
	}
}

class Stickman {
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
	// Segments are lines connecting two pivots
	// These are the lengths of each segment
	float SHANK, THIGH, LOWERBODY, UPPERBODY, HEAD, UPPERARM, LOWERARM;
	private static float JOINTHRESH=0.3f;   // How close two hands have to be to join
	private static float BREAKTHRESH=2.0f;
	private static float DOTRADIUS=0.05f;   // Radius of hand-hold dot
	
	int id;
	float height;
	PVector hipPosition;
	Stickman partner[];
	int timeConnected;
	
	private static HashSet<Stickman> allStickmen = new HashSet<Stickman>();
	
	Stickman(float height, int id){
		SHANK=height/5;THIGH=height/5;
		LOWERBODY=height/5;UPPERBODY=height/5;HEAD=height/5;
		UPPERARM=height/5;LOWERARM=height/5;
		hipPosition=new PVector(0,0);
		this.height=height;
		partner=new Stickman[2];
		allStickmen.add(this);
		this.id=id;
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
		//TOP=10*(float)Math.sin(Tracker.theTracker.frameCount/20.0*2*Math.PI); 
     
		float signalLevel=Math.abs(Tracker.theTracker.fourier.left[20]);
		PApplet.println("signal="+signalLevel);
		TOP= signalLevel*200;
		
		// Accomodate any hand holds
		for (int i=0;i<partner.length;i++) {
			if (partner[i]!=null) {
				PVector myHand=getHandPosition(i);
				int j = partner[i].getHand(this);
				PVector otherHand = partner[i].getHandPosition(j);
				if (PVector.dist(myHand, otherHand)>JOINTHRESH/10) {
					// Attempt to bend to keep in the right place
					//PVector target=PVector.mult(PVector.add(myHand, otherHand), 0.5f);
					//if (id<partner[i].id)
						moveToHandPosition(i,otherHand);
					//partner[i].moveToHandPosition(j,target);
				}
				// Check if we're still too far apart
				if (PVector.dist(myHand, otherHand)>BREAKTHRESH) {
					// Split up
					partner[i].partner[j]=null;
					partner[i]=null;
				}
					
			}
		}
	}
	
	// Return false if too far apart
	boolean moveToHandPosition(int which, PVector newpos) {
		// Attempt to move figure so that hand[which] is at newpos
		// Don't move feet though
		PVector delta=PVector.sub(newpos, getNeckPosition());
		float r=delta.mag();
		double hand;
		double diff;
		if (r<UPPERARM+LOWERARM) {
			NECK++;
			delta=PVector.sub(newpos, getNeckPosition());
			if (delta.mag() > r) {
				NECK-=2;
				delta=PVector.sub(newpos, getNeckPosition());
			}
			r=delta.mag();
		}
		if (r<UPPERARM+LOWERARM) {
			 hand=-180+Math.acos((UPPERARM*UPPERARM+LOWERARM*LOWERARM-r*r)/(2*UPPERARM*LOWERARM))*180/Math.PI;
			 diff=Math.acos((UPPERARM*UPPERARM-LOWERARM*LOWERARM+r*r)/(2*LOWERARM*r))*180/Math.PI;
			// Angle from shoulder to hand:
		} else {
			hand=0;
			diff=0;
		}
		double theta=Math.atan2(delta.x,-delta.y)*180/Math.PI;
		double elbow=theta+diff;
		if (which==0) {
			LELBOW=(float)elbow;
			LHAND=(float)hand;
		} else {
			RELBOW=-(float)elbow;
			RHAND=-(float)hand;
		}
		PApplet.println("delta="+delta,", r="+r+", hand="+hand+", diff="+diff+", theta="+theta+", elbow="+elbow);
		return hand==0;
	}
	
	PVector getHandPosition(int which) {
		PVector hand=getNeckPosition();
		float upperArmAngle, lowerArmAngle;
		if (which==0) {
			upperArmAngle=WAIST+NECK+LELBOW;
			lowerArmAngle=upperArmAngle+LHAND;
		} else {
			upperArmAngle=WAIST+NECK-RELBOW;
			lowerArmAngle=upperArmAngle-RHAND;
		}
		hand.x+=(float)(UPPERARM*Math.sin(radians(upperArmAngle))+LOWERARM*Math.sin(radians(lowerArmAngle)));
		hand.y-=(float)(UPPERARM*Math.cos(radians(upperArmAngle))+LOWERARM*Math.cos(radians(lowerArmAngle)));
		return hand;
	}
	
	PVector getNeckPosition() {
		PVector neck=hipPosition.copy();
		neck.x+=(float)(LOWERBODY*Math.sin(radians(WAIST))+UPPERBODY*Math.sin(radians(WAIST+NECK)));
		neck.y-=(float)(LOWERBODY*Math.cos(radians(WAIST))+UPPERBODY*Math.cos(radians(WAIST+NECK)));
		return neck;
	}
	
	int getHand(Stickman who) {
		for (int i=0;i<partner.length;i++)
			if (partner[i]==who)
				return i;
		return 0;
	}
	
	void destroy() {
		allStickmen.remove(this);
		// Remove any (reciprocal) hand-holding
		for (int i=0;i<partner.length;i++) {
			if (partner[i]!=null) {
				for (int j=0;i<partner[j].partner.length;j++) {
					if (partner[i].partner[j]!=null && partner[i].partner[j]==this)
						partner[i].partner[j]=null;
				}
			}
		}
	}
	
	float radians(float angle) { return PApplet.radians(angle); }
	
	void draw(PGraphics g) {
		if (partner[0]!=null || partner[1]!=null)
			timeConnected++;
		else
			timeConnected=0;
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
			if (partner[0]!=null)
				g.arc(0, LOWERARM, DOTRADIUS, DOTRADIUS, 0, radians(360));
			g.popMatrix();
		}
		g.pushMatrix(); {
			g.rotate(radians(-RELBOW));
			g.line(0f, 0f, 0f, UPPERARM);
			g.translate(0f, UPPERARM);
			g.rotate(radians(-RHAND));
			g.line(0f,0f,0f,LOWERARM);
			if (partner[1]!=null)
				g.arc(0, LOWERARM, DOTRADIUS, DOTRADIUS, 0, radians(360));
			g.popMatrix();
		}
		g.rotate(radians(TOP));
		g.translate(0f, HEAD/2);
		g.ellipseMode(PConstants.CENTER);
		g.arc(0f, 0f, HEAD, HEAD, 0, radians(360));
		float smileAngle=timeConnected/2+5;
		if (smileAngle>50)
			smileAngle=50;
		g.arc(0f, 0f, HEAD/2, HEAD/2, radians(-smileAngle-90), radians(smileAngle-90));

		g.popMatrix();
		for (int i=0;i<2;i++) {
			PVector h=getHandPosition(i);
			g.arc(h.x, h.y, DOTRADIUS/2, DOTRADIUS/2, 0, radians(180));
		}
	}
	
	void connectIfClose(Stickman other) {
		// If either hand of the other stickman is free and close enough, connect them
		for (int myleft=0;myleft<partner.length;myleft++) {
			if (partner[myleft]!=null)
				continue;  // Already holding someone
			PVector myhand=getHandPosition(myleft);
			for (int otherleft=0;otherleft<other.partner.length;otherleft++) {
				if (other.partner[otherleft]!=null)
					continue; // Already holding someone
				PVector otherHand=other.getHandPosition(otherleft);	
				if (PVector.dist(otherHand, myhand)<JOINTHRESH) {
					partner[myleft]=other;
					other.partner[otherleft]=this;
				}
			}
		}
	}
	
	static void updateAll(Effects effects) {
		// Find any new handholds
		for (Stickman s1:allStickmen)
			for (Stickman s2:allStickmen) {
				if (s1!=s2)
					s1.connectIfClose(s2);
			}		
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
				sticks.put(id,new Stickman(2.0f,id));
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

