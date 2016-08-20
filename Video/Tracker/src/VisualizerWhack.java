import java.util.ArrayList;
import java.util.List;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

// Class for a single mole 
class Mole {
	PVector position=null;
	PVector velocity=null;
	
	PImage img;
	Images moleImages, whackedImages;
	
	int explodeCounter;
	int ticksSinceReset=0;
	static final int explodeFrames=50;
	static final float maxExplodeScale=5f;
	static final float RESETPROB=0.04f;
	static final int MINLIFE=30;  // Minimum number of ticks before reset
	static final float MAXSPEED=1f/100;   // Maximum speed in meters/tick
	static final float maxHitDist=0.4f; // Meters
	static final float radius=0.3f;  // Meters
	
	Mole(Images m,Images w) { 
		this.moleImages=m;
		this.whackedImages=w;
		this.img=null;
		explodeCounter=0;
	}
	
	void draw(PGraphics g) {
		if (position==null)
			return;
		g.pushStyle();
		g.imageMode(PConstants.CENTER);
		float r=radius;
		if (explodeCounter>0)
			g.tint(255,255*explodeCounter/explodeFrames);
		Visualizer.drawImage(g,img,position.x, position.y,r*2, r*2);
		g.popStyle();
	}
	
	void reset(People p) {
		// Choose new location for mole
		if (position==null)
			position=new PVector();
		if (velocity==null)
			velocity=new PVector();
		position.y=(float) (Math.random()*(Tracker.maxy-Tracker.miny)+Tracker.miny); 
		position.x=(float) (Math.random()*(Tracker.maxx-Tracker.minx)+Tracker.minx);
		velocity.y=(float) ((Math.random()*2-1)*MAXSPEED);
		velocity.x=(float) ((Math.random()*2-1)*MAXSPEED);
		ticksSinceReset=0;
		img=moleImages.getRandom();
	}
	
	void update(People p, Effects e) {
		if (explodeCounter>0) {
			explodeCounter-=1;
			if (explodeCounter==0)
				reset(p);
			else
				return;
		} else 
			ticksSinceReset+=1;
		
		if (position==null || (ticksSinceReset>MINLIFE && Math.random()<RESETPROB))
			reset(p);
		else
			position.add(velocity);

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
					if (nclips!=-1) {
						int clip=(int)(Math.random()*nclips);
						PApplet.println("Track="+track+", nclips="+nclips+", clip="+clip);
						Ableton.getInstance().playClip(track,clip);
					}
					e.play("WHACK", 127, 1000);
					img=whackedImages.getRandom();
				}
			}
		}
	}
}

public class VisualizerWhack extends VisualizerIcon {
	List<Mole> moles = new ArrayList<Mole>();
	Images moleImages;
	Images whackedImages;
	final String hammerDir="hammers";
	final String moleDir="moles";
	final String whackedDir="whacked";
	String trackSet;
	static final int numMoles=3;
	Effects effects;
	
	VisualizerWhack(PApplet parent, String dataDir, String trackSet) {
		super(parent);
		setImages(parent,dataDir+"/"+hammerDir);
		moleImages=new Images(dataDir+"/"+moleDir);
		whackedImages=new Images(dataDir+"/"+whackedDir);
		for (int i=0;i<numMoles;i++) {
			moles.add(new Mole(moleImages,whackedImages));
		}
		this.effects=Effects.defaultEffects;
		this.trackSet=trackSet;
	}
	
	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet(trackSet);
	}
	
	@Override
	public void update(PApplet parent, People p) {
		super.update(parent, p);
		for (Mole m: moles) 
			m.update(p,effects);
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
