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
	
	static final float speed=0.04f;  // Meters/frame
	static final float maxHitDist=0.4f; // Meters
	static final float appleRadius=0.3f;  // Meters
	
	Apple(PVector pos) { position=pos; }
	
	void drawLaser() {
		Laser laser=Laser.getInstance();
		laser.svgfile("apple.svg",position.x,position.y,0.5f,0f);
	}
	void draw(PGraphics g) {
//		if (appleShape==null)
//			appleShape=g.loadShape(Tracker.SVGDIRECTORY+"apple.svg");
		if (appleImage==null)
			appleImage=Tracker.theTracker.loadImage("49728.gif");
//		PApplet.println("Drawing apple shape at "+p);
//		g.shapeMode(PConstants.CENTER);
//		Visualizer.drawShape(g, appleShape,position.x, position.y,appleRadius*2, appleRadius*2);
		g.imageMode(PConstants.CENTER);
		g.image(appleImage,position.x, position.y,appleRadius*2, appleRadius*2);
	}
	
	void update(People p) {
		final float minsize=0.1f;
		final float maxsize=2.0f;
		position.y=position.y+speed;
		boolean hit=false;
		if (position.y>Tracker.rawmaxy) {
			// Hit bottom
			hit=true;
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
				PApplet.println("Person "+ps.id+" has size "+ps.userData);
				TrackSet ts=Ableton.getInstance().trackSet;
				int track=ps.id%(ts.numTracks)+ts.firstTrack;
				int nclips=Ableton.getInstance().getTrack(track).numClips();
				PApplet.println("Track="+track+", nclips="+nclips);
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
//		PApplet.println("apple position="+position+", hit="+hit);
		if (hit) {
			position.y=-1.0f;  // Give it some blanking time
			position.x=(float) (Math.random()*(Tracker.rawmaxx-Tracker.rawminx)+Tracker.rawminx);
		}
	}
}

public class VisualizerCows extends VisualizerIcon {
	Apple apple;
	
	final String cowIcons[]={"cow1.svg","cow2.svg","cow3.svg","ToastingCow002.svg"};
	final String cowImages[]={"cow_PNG2125.png","cow_PNG2127.png","cow_PNG2130.png","cow_PNG2138.png","cow_PNG2140.png","cow_PNG2141.png"};
	VisualizerCows(PApplet parent) {
		super(parent);
		setIcons(parent,cowIcons);
		setImages(parent,cowImages);
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

		g.background(0);
		g.shapeMode(PApplet.CENTER);
		apple.draw(g);
		
		for (Person ps: p.pmap.values()) {  
			final float sz=0.30f+0.60f*2*ps.userData;  // Size to make the icon's largest dimension, in meters

			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			if (useImages) {
				assert(images.length>0);
				PImage img=images[ps.id%images.length];
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
				//			PApplet.println("Display shape "+icon+" with native size "+icon.width+","+icon.height);
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
