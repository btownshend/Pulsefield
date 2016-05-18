import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PShape;
import processing.core.PVector;

class Apple {
	PVector position;
	PShape appleShape;
	int nextClip=0;
	
	static final float speed=0.04f;  // Meters/frame
	static final float maxHitDist=0.2f; // Meters
	static final float appleRadius=0.3f;  // Meters
	
	Apple(PVector pos) { position=pos; }
	
	void drawLaser() {
		Laser laser=Laser.getInstance();
		laser.svgfile("apple.svg",position.x,position.y,0.5f,0f);
	}
	void draw(PGraphics parent, PVector wsize) {
		if (appleShape==null)
			appleShape=parent.loadShape(Tracker.SVGDIRECTORY+"apple.svg");
//		PApplet.println("Drawing apple shape at "+p);
		parent.shapeMode(PConstants.CENTER);
		parent.shape(appleShape,position.x, position.y,appleRadius*2, appleRadius*2);
	}
	
	void update(People p) {
		position.y=position.y+speed;
		boolean hit=false;
		if (position.y>Tracker.rawmaxy) {
			// Hit bottom
			hit=true;
		}
		for (Person ps: p.pmap.values()) {
			if (ps.userData==0)
				// Initialize
				ps.userData=0.5f;
			float d=PVector.dist(position, ps.getOriginInMeters());
			if (d<maxHitDist) {
				hit=true;
				ps.userData+=0.1f;
				PApplet.println("Person "+ps.id+" has size "+ps.userData);
				TrackSet ts=Ableton.getInstance().trackSet;
				int track=ps.id%(ts.numTracks)+ts.firstTrack;
				int nclips=Ableton.getInstance().getTrack(track).numClips();
				PApplet.println("Track="+track+", nclips="+nclips);
				if (nclips!=-1) {
					Ableton.getInstance().playClip(track,nextClip);
					nextClip=(nextClip+1)%nclips;
				}
			} else {
				ps.userData-=0.001f;
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
	VisualizerCows(PApplet parent) {
		super(parent);
		setIcons(parent,cowIcons);
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
	public void draw(Tracker t, PGraphics g, People p, PVector wsize) {
		if (p.pmap.isEmpty()) {
			super.draw(t, g, p, wsize);
			return;
		}

		g.background(127);
		g.shapeMode(PApplet.CENTER);
		apple.draw(g,wsize);

		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			PShape icon=iconShapes[ps.id%iconShapes.length];
			//icon.translate(-icon.width/2, -icon.height/2);
//			PApplet.println("Display shape "+icon+" with native size "+icon.width+","+icon.height);
			final float sz=0.30f+0.60f*2*ps.userData;  // Size to make the icon's largest dimension, in pixels
			
			float scale=Math.min(sz/icon.width,sz/icon.height);
			g.shape(icon,ps.getOriginInMeters().x, ps.getOriginInMeters().y,icon.width*scale,icon.height*scale);
			//icon.resetMatrix();
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
