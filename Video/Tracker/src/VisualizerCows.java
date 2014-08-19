import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PShape;
import processing.core.PVector;

class Apple {
	PVector position;
	PShape appleShape;
	
	static final float speed=0.04f;  // Meters/frame
	static final float maxHitDist=0.2f; // Meters
	static final float appleRadius=0.3f;  // Meters
	static final float applePixels=30;
	
	Apple(PVector pos) { position=pos; }
	
	void drawLaser() {
		Laser laser=Laser.getInstance();
		laser.svgfile("apple.svg",position.x,position.y,0.5f,0f);
	}
	void draw(PApplet parent, PVector wsize) {
		if (appleShape==null)
			appleShape=parent.loadShape(Tracker.SVGDIRECTORY+"apple.svg");
		PVector p=Tracker.floorToNormalized(position);
//		PApplet.println("Drawing apple shape at "+p);
		parent.shape(appleShape,((p.x+1)*wsize.x)/2, ((p.y+1)*wsize.y)/2,applePixels, applePixels);
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
				if (nclips!=-1)
					Ableton.getInstance().playClip(track,ps.id%nclips);
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
	public void draw(PApplet parent, People p, PVector wsize) {
		if (p.pmap.isEmpty()) {
			super.draw(parent,p, wsize);
			return;
		}

		parent.background(127);
		parent.shapeMode(PApplet.CENTER);
		apple.draw(parent,wsize);

		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			PShape icon=iconShapes[ps.id%iconShapes.length];
			//icon.translate(-icon.width/2, -icon.height/2);
//			PApplet.println("Display shape "+icon+" with native size "+icon.width+","+icon.height);
			final float sz=30+60*2*ps.userData;  // Size to make the icon's largest dimension, in pixels
			
			float scale=Math.min(sz/icon.width,sz/icon.height);
			parent.shape(icon,(ps.getNormalizedPosition().x+1)*wsize.x/2, (ps.getNormalizedPosition().y+1)*wsize.y/2,icon.width*scale,icon.height*scale);
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
