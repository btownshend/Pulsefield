import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PShape;

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

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		g.background(127);
		g.shapeMode(PApplet.CENTER);
		final float sz=60;  // Size to make the icon's largest dimension, in pixels
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			PShape icon=iconShapes[ps.id%iconShapes.length];
			//icon.translate(-icon.width/2, -icon.height/2);
//			PApplet.println("Display shape "+icon+" with native size "+icon.width+","+icon.height);
			float scale=Math.min(sz/icon.width,sz/icon.height);
			g.shape(icon,ps.getOriginInMeters().x, ps.getOriginInMeters().y,icon.width*scale,icon.height*scale);
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

