import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PShape;
import processing.core.PVector;

// Visualizer that just displays a dot for each person

public class VisualizerIcon extends Visualizer {
	String icons[];
	PShape iconShapes[];
	
	VisualizerIcon(PApplet parent) {
		super();
	}
	
	public void setIcons(PApplet parent, String ic[]) {
		this.icons=ic;
		iconShapes=new PShape[icons.length];
		for (int i=0;i<iconShapes.length;i++) {
			iconShapes[i]=parent.loadShape(Tracker.SVGDIRECTORY+icons[i]);
			assert(iconShapes[i]!=null);
			PApplet.println("Loaded "+icons[i]+" with "+iconShapes[i].getChildCount()+" children, size "+iconShapes[i].width+"x"+iconShapes[i].height);
		}
	}
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}
	
	public void stop() {
		super.stop();
	}

	public void update(PApplet parent, People p) {
		;
	}

	public void draw(PGraphics parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		if (p.pmap.isEmpty())
			return;
		parent.background(127);
		parent.shapeMode(PApplet.CENTER);
		final float sz=60;  // Size to make the icon's largest dimension, in pixels
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			PShape icon=iconShapes[ps.id%iconShapes.length];
			//icon.translate(-icon.width/2, -icon.height/2);
//			PApplet.println("Display shape "+icon+" with native size "+icon.width+","+icon.height);
			float scale=Math.min(sz/icon.width,sz/icon.height);
			parent.shape(icon,(ps.getNormalizedPosition().x+1)*wsize.x/2, (ps.getNormalizedPosition().y+1)*wsize.y/2,icon.width*scale,icon.height*scale);
			//icon.resetMatrix();
		}
	}
	
	public void drawLaser(PApplet parent, People p) {
		Laser laser=Laser.getInstance();
		for (Person ps: p.pmap.values()) {  
			String icon=icons[ps.id%icons.length];
			laser.cellBegin(ps.id);
			laser.svgfile(icon,0.0f,0.0f,0.7f,0.0f);
			laser.cellEnd(ps.id);
		}
	}
}

