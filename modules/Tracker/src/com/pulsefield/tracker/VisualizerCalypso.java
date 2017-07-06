package com.pulsefield.tracker;
import java.util.ArrayList;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

class Drum {
	PVector center;
	PImage img;
	float radius;
	ArrayList<NoteSpot> noteSpots; 
	final boolean drawOutlines=true;
	
	Drum(PVector center, float radius, PImage img) {
		this.center=center;
		this.img=img;
		this.noteSpots=new ArrayList<NoteSpot>();
		this.radius=radius;
	}
	void addNote(String note, float radius, float angle, float sz) {
		PVector pos=new PVector((float)(center.x+radius*Math.cos(angle)),(float)(center.y+radius*Math.sin(angle)));
		noteSpots.add(new NoteSpot(Synth.nameToPitch(note),127,1000,pos,sz));
	}
	@SuppressWarnings("unused")
	void draw(PGraphics g) {
		g.pushStyle();
		g.imageMode(PConstants.CENTER);
		g.pushMatrix();
		g.translate(center.x, center.y);
		g.scale(-1f,1f);
		float width;
		float height;
		if (img!=null && !drawOutlines) {
			if (img.width<img.height) {
				width=radius*2;
				height=width*img.height/img.width;
			} else {
				height=radius*2;
				width=height*img.width/img.height;
			}
			g.image(img,0f,0f,width, height);
		}
		g.popMatrix();
		if (drawOutlines) {
			g.noFill();
			g.strokeWeight(0.05f);
			g.stroke(50);
			g.ellipseMode(PConstants.CENTER);
			g.ellipse(center.x,center.y,radius*2,radius*2);

			g.strokeWeight(0.03f);
			g.textAlign(PConstants.CENTER,PConstants.CENTER);
			for (NoteSpot n: noteSpots) {
				if (n.activeFrames>0) {
					g.stroke(0,255,0);
					n.activeFrames--;
				} else
					g.stroke(70);
				g.ellipse(n.pos.x, n.pos.y, n.radius*2, n.radius*2);
				Visualizer.drawText(g, n.radius/2, Synth.pitchToName(n.pitch), n.pos.x, n.pos.y);			
			}
		}
		g.popStyle();
	}
}
class LeadDrum extends Drum {
	LeadDrum(PVector pos, float radius) {
		super(pos,radius,Tracker.theTracker.loadImage("calypso/pan.png"));
		final float innerRadius=0.25f*radius;
		float a0=(float)Math.PI*0.29f;
		float angleStep=(float)Math.PI*2/4;
		float sz=radius/8;
		addNote("E6", innerRadius, a0+0*angleStep, sz);
		addNote("F6", innerRadius, a0+1*angleStep, sz);
		addNote("D6", innerRadius, a0+2*angleStep, sz);
		addNote("G6", innerRadius, a0+3*angleStep, sz);
		final float outerRadius=0.70f*radius;
		a0=(float)(Math.PI*0.70f);
		angleStep=(float)Math.PI*2/8;
		sz=(float)(2*Math.PI*outerRadius/8)/2.1f;
		addNote("C5", outerRadius, a0+0*angleStep, sz);
		addNote("A5", outerRadius, a0+1*angleStep, sz);
		addNote("F5", outerRadius, a0+2*angleStep, sz);
		addNote("D5", outerRadius, a0+3*angleStep, sz);
		addNote("B5", outerRadius, a0+4*angleStep, sz);
		addNote("G5", outerRadius, a0+5*angleStep, sz);
		addNote("E5", outerRadius, a0+6*angleStep, sz);
		addNote("C6", outerRadius, a0+7*angleStep, sz);
	}
}

public class VisualizerCalypso extends Visualizer {
	Drum drums[];
	PVector center;
	Synth synth;
	
	VisualizerCalypso(PApplet parent, Synth synth) {
		super();
		this.synth=synth;
		drums=new Drum[2];
		PVector sz=Tracker.getFloorSize();
		float dradius=Math.min(sz.x,sz.y)/4;
		drums[0]=new LeadDrum(PVector.add(Tracker.getFloorCenter(),new PVector(dradius*0.9f,dradius*0.6f)),dradius*0.9f);
		drums[1]=new LeadDrum(PVector.add(Tracker.getFloorCenter(),new PVector(-dradius*0.9f,-dradius*0.6f)),dradius*0.9f);
	}

	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("SteelPan");
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		int oldPitch[]=new int[2];
		int newPitch[]=new int[2];

		for (Person ps: p.pmap.values()) {  
			oldPitch[0]=((int)ps.userData)%1000;
			oldPitch[1]=((int)ps.userData)/1000;
			newPitch[0]=0;
			newPitch[1]=0;
			for (Drum d: drums) {
				for (NoteSpot noteSpot: d.noteSpots) {	
					for (int i=0;i<ps.legs.length;i++) {
						Leg leg=ps.legs[i];
						if (noteSpot.hitTest(leg.getOriginInMeters())) {
							//logger.fine("Hit note "+noteSpot.pitch+", oldPitch="+oldPitch[i]);
							newPitch[i]=noteSpot.pitch;
							if (oldPitch[i]!=noteSpot.pitch) {
								// New pitch, trigger it
								logger.fine("Play note "+noteSpot.pitch+", vel="+noteSpot.velocity+", dur="+noteSpot.duration);
								synth.play(ps.id, noteSpot.pitch, noteSpot.velocity, noteSpot.duration, ps.channel);
								noteSpot.setActive((int)(noteSpot.duration*parent.frameRate/1000));
							}
						}
					}
				}
				ps.userData=newPitch[0]+newPitch[1]*1000;
			}
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty()) {
			return;
		}
		for (Drum d: drums) 
			d.draw(g);
		g.pushStyle();
		g.strokeWeight(0.02f);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			for (int i=0;i<ps.legs.length;i++) {
				int curNote=(i==0)?((int)ps.userData)%1000:((int)ps.userData)/1000;  // Current pitch for this leg
				if (curNote==0)
					g.stroke(63);
				else
					g.stroke(0,255,0);
				Leg leg=ps.legs[i];
				PVector pos=leg.getOriginInMeters();
				g.ellipse(pos.x, pos.y, leg.getDiameterInMeters(), leg.getDiameterInMeters());
			}
		}
		g.popStyle();
	}
	
}
