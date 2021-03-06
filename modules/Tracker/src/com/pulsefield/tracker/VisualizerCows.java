package com.pulsefield.tracker;
import java.util.logging.Logger;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PShape;
import processing.core.PVector;

class Apple {
	PVector position;
	PShape appleShape;
	PImage appleImage;
	int nextClip=0;
	int entrySide=0;  // 0-top,1-right,2-bottom,3-left
	static final float speed=0.04f;  // Meters/frame
	static final float maxHitDist=0.4f; // Meters
	static final float appleRadius=0.3f;  // Meters
    private final static Logger logger = Logger.getLogger(Apple.class.getName());

	Apple(PVector pos) { position=pos; }
	
	void drawLaser() {
		Laser laser=Laser.getInstance();
		laser.svgfile("apple.svg",position.x,position.y,0.5f,0f);
	}
	void draw(PGraphics g) {
//		if (appleShape==null)
//			appleShape=g.loadShape(Tracker.SVGDIRECTORY+"apple.svg");
		if (appleImage==null)
			appleImage=Tracker.theTracker.loadImage("cows/49728.gif");
//		logger.fine("Drawing apple shape at "+p);
//		g.shapeMode(PConstants.CENTER);
//		Visualizer.drawShape(g, appleShape,position.x, position.y,appleRadius*2, appleRadius*2);
		g.pushMatrix();
		g.imageMode(PConstants.CENTER);
		g.translate(position.x, position.y);
		g.rotate((float)(-entrySide*Math.PI/2));
		g.image(appleImage,0f,0f,appleRadius*2, appleRadius*2);
		g.popMatrix();
	}
	
	void update(People p) {
		final float minsize=0.1f;
		final float maxsize=2.0f;
		switch (entrySide) {
			case 0:
				position.y=position.y+speed;
				break;
			case 1:
				position.x=position.x+speed;
				break;
			case 2:
				position.y=position.y-speed;
				break;
			case 3:
				position.x=position.x-speed;
				break;
		}
		boolean hit=false;
		switch (entrySide) {
		case 0:
			hit=position.y>Tracker.maxy;
			break;
		case 1:
			hit=position.x>Tracker.maxx;
			break;	
		case 2:
			hit=position.y<Tracker.miny;
			break;
		case 3:
			hit=position.x<Tracker.minx;
			break;	
		}

		int numhits=0;
		for (Person ps: p.pmap.values()) {
			float d=PVector.dist(position, ps.getOriginInMeters());
			if (d<maxHitDist) {
				numhits+=1;
			}
		}
		for (Person ps: p.pmap.values()) {
			if (ps.userData==0)
				// Initialize
				ps.userData=0.5f;
			float d=PVector.dist(position, ps.getOriginInMeters());
			if (d<maxHitDist) {
				hit=true;
				ps.userData=(ps.userData+maxsize)/2;
				logger.fine("Person "+ps.id+" has size "+ps.userData);
				TrackSet ts=Ableton.getInstance().trackSet;
				int track=ps.id%(ts.numTracks)+ts.firstTrack;
				int nclips=Ableton.getInstance().getTrack(track).numClips();
				logger.fine("Track="+track+", nclips="+nclips);
				if (nclips!=-1) {
					Ableton.getInstance().playClip(track,nextClip);
					nextClip=(nextClip+1)%nclips;
				}
			} else if (numhits>0) {
				ps.userData=(ps.userData+minsize)/2;
			} else {
				;
			}
			ps.userData=Math.max(0.0f, Math.min(1.0f, ps.userData));
		}
//		logger.fine("apple position="+position+", hit="+hit);
		if (hit) {
			entrySide=(int)(Math.random()*4);
			switch (entrySide) {
			case 0:
				position.y=Tracker.miny-1.0f;  // Give it some blanking time
				position.x=(float) (Math.random()*(Tracker.maxx-Tracker.minx)+Tracker.minx);
				break;
			case 1:
				position.x=Tracker.minx-1.0f;  // Give it some blanking time
				position.y=(float) (Math.random()*(Tracker.maxy-Tracker.miny)+Tracker.miny);
				break;
			case 2:
				position.y=Tracker.maxy+1.0f;  // Give it some blanking time
				position.x=(float) (Math.random()*(Tracker.maxx-Tracker.minx)+Tracker.minx);
				break;
			case 3:
				position.x=Tracker.maxx+1.0f;  // Give it some blanking time
				position.y=(float) (Math.random()*(Tracker.maxy-Tracker.miny)+Tracker.miny);
				break;
			}
		}
	}
}

public class VisualizerCows extends VisualizerIcon {
	Apple apple;
	
	final String cowIcons[]={"cow1.svg","cow2.svg","cow3.svg","ToastingCow002.svg"};
	final String cowImageDir="cows/cows";
	VisualizerCows(PApplet parent) {
		super(parent);
		setIcons(parent,cowIcons);
		setImages(parent,cowImageDir);
		apple=new Apple(new PVector(0f,0f));
	}
	
	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Cows");
	}
	
	@Override
	public void update(PApplet parent, People p) {
		super.update(parent, p);
		apple.update(p);
	}
	
	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		if (p.pmap.isEmpty()) {
			super.draw(t, g, p);
			return;
		}
		super.draw(t, g, p);
		g.background(0);
		g.shapeMode(PApplet.CENTER);
		apple.draw(g);
		
		for (Person ps: p.pmap.values()) {  
			final float sz=0.30f+0.60f*2*ps.userData;  // Size to make the icon's largest dimension, in meters

			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			if (useImages) {
				PImage img=images.get(ps.id);
				float scale=Math.min(sz/img.width,sz/img.height);
				g.pushMatrix();
				g.translate(ps.getOriginInMeters().x, ps.getOriginInMeters().y);
				if (ps.getVelocityInMeters().x >0.1)
					g.scale(-1,1);
				g.image(img,0,-sz/4,img.width*scale,img.height*scale);
				g.popMatrix();
			} else {
				PShape icon=iconShapes[ps.id%iconShapes.length];
				//icon.translate(-icon.width/2, -icon.height/2);
				//			logger.fine("Display shape "+icon+" with native size "+icon.width+","+icon.height);
				float scale=Math.min(sz/icon.width,sz/icon.height);
				Visualizer.drawShape(g, icon,ps.getOriginInMeters().x, ps.getOriginInMeters().y-icon.height*scale/2,icon.width*scale,icon.height*scale);
				//icon.resetMatrix();
			}
		}
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		Laser laser=Laser.getInstance();
		laser.bgBegin();
		// Draw falling apple
		laser.shapeBegin("apple");
		apple.drawLaser();
		laser.shapeEnd("apple");
		laser.bgEnd();
		

		for (Person ps: p.pmap.values()) {  
			String icon=icons[ps.id%icons.length];
			laser.cellBegin(ps.id);
			laser.svgfile(icon,0.0f,0.0f,ps.userData,0.0f);
			laser.cellEnd(ps.id);
		}
	}
}
