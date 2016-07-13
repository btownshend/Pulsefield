import java.util.ArrayList;
import java.util.List;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PShape;
import processing.core.PVector;

// Class for a single mole 
class Mole {
	PVector position=null;
	PImage img;
	int nextClip=0;
	int explodeCounter;
	static final int explodeFrames=30;
	
	static final float maxHitDist=0.4f; // Meters
	static final float radius=0.3f;  // Meters
	
	Mole(PImage img) { 
		this.img=img;
		explodeCounter=0;
		}
	
	void draw(PGraphics g) {
		if (position==null)
			return;
		g.imageMode(PConstants.CENTER);
		float r=radius;
		if (explodeCounter > 0)
			r=radius*(1+3f*(explodeFrames-explodeCounter)/explodeFrames);
		g.image(img,position.x, position.y,r*2, r*2);
	}
	
	void reset(People p) {
		// Choose new location for mole
		if (position==null)
			position=new PVector();
		position.y=(float) (Math.random()*(Tracker.rawmaxy-Tracker.rawminy)+Tracker.rawminy); 
		position.x=(float) (Math.random()*(Tracker.rawmaxx-Tracker.rawminx)+Tracker.rawminx);
	}
	
	void update(People p) {
		if (explodeCounter>0) {
			explodeCounter-=1;
			if (explodeCounter==0)
				reset(p);
			else
				return;
		}
		
		if (position==null)
			reset(p);
		
		for (Person ps: p.pmap.values()) {
			float d=PVector.dist(position, ps.getOriginInMeters());
			if (d<maxHitDist) {
				explodeCounter=explodeFrames;
				ps.userData+=1;
				PApplet.println("Person "+ps.id+" hit, score: "+ps.userData);
				TrackSet ts=Ableton.getInstance().trackSet;
				if (ts==null)
					PApplet.println("No track for whack");
				else {
					int track=ps.id%(ts.numTracks)+ts.firstTrack;
					int nclips=Ableton.getInstance().getTrack(track).numClips();
					PApplet.println("Track="+track+", nclips="+nclips);
					if (nclips!=-1) {
						Ableton.getInstance().playClip(track,nextClip);
						nextClip=(nextClip+1)%nclips;
					}
				}
			}
		}
	}
}

public class VisualizerWhack extends VisualizerIcon {
	List<Mole> moles = new ArrayList<Mole>();
	Images moleImages;
	final String hammerDir="data/whack/hammers";
	final String moleDir="data/whack/moles";
	static final int numMoles=4;
	
	VisualizerWhack(PApplet parent) {
		super(parent);
		setImages(parent,hammerDir);
		moleImages=new Images(moleDir);
		for (int i=0;i<numMoles;i++) {
			moles.add(new Mole(moleImages.getRandom()));
		}
	}
	
	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Whack");
	}
	
	@Override
	public void update(PApplet parent, People p) {
		super.update(parent, p);
		for (Mole m: moles) 
			m.update(p);
	}
	
	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		
		g.background(0);
		g.shapeMode(PApplet.CENTER);
		for (Mole m: moles) 
			m.draw(g);
		
		for (Person ps: p.pmap.values()) {  
			final float sz=0.30f+0.05f*ps.userData;  // Size to make the icon's largest dimension, in meters

			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			PImage img=images.get(ps.id);
			float scale=Math.min(sz/img.width,sz/img.height);
			g.pushMatrix();
			g.translate(ps.getOriginInMeters().x, ps.getOriginInMeters().y);
			if (ps.getVelocityInMeters().x >0.1)
				g.scale(-1,1);
			g.image(img,0,-sz/4,img.width*scale,img.height*scale);
			g.popMatrix();
		}
	}
}
