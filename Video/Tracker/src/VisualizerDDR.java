import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

class Dancer {
	final float DAMPING=.02f;
	PVector neutral;
	PVector current;
	Dancer(PVector pos) {
		neutral=new PVector();
		neutral.x=pos.x; neutral.y=pos.y;
		current=new PVector();
		current.x=pos.x; current.y=pos.y;
	}
	public void update(PVector newpos) {
		neutral.x=DAMPING*newpos.x+(1-DAMPING)*neutral.x;
		neutral.y=DAMPING*newpos.y+(1-DAMPING)*neutral.y;
		current.x=newpos.x;
		current.y=newpos.y;
	}
}

enum Direction {
	LEFT, RIGHT, UP, DOWN
}

// Dance revolution visualizer
public class VisualizerDDR extends Visualizer {
	final float ARROWDIST=20f;
	final float MINMOVEDIST=.01f;
	Simfile sf;
	PImage banner;
	long startTime;
	PImage arrow;

	HashMap<Integer, Dancer> dancers;
	Simfile moves[];

	VisualizerDDR(PApplet parent) {
		super();
		dancers = new HashMap<Integer, Dancer>();
		arrow = parent.loadImage("arrow.png");
		sf = new Simfile();
		try {
//			sf.loadSM("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/StepMix 1.0/Impossible Fidelity/","impossible.sm");
			sf.loadSM("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/Plaguemix Series/Super Trouper","supertrouper.sm");
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println(sf.toString());
		banner=sf.getBanner(parent);
	}

	public void update(PApplet parent, Positions allpos) {
		// Update internal state of the dancers
		for (int id: allpos.positions.keySet()) {
			if (!dancers.containsKey(id))
				dancers.put(id,new Dancer(allpos.get(id).origin));
			PVector currentpos=allpos.get(id).origin;
			dancers.get(id).update(currentpos);
			//PApplet.println("Dancer "+id+" moved to "+currentpos.toString());
		}
		// Remove dancers for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = dancers.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.positions.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				iter.remove();
			}
		}
	}

	public void beat() {
		// Called each time a beat passes by
	}

	public void start() {
		startTime=System.currentTimeMillis();
		PApplet.println("Starting DDR at "+startTime);
		Ableton.getInstance().playClip(91,1);
	}
	
	public void stop() {
		PApplet.println("Stopping DDR at "+System.currentTimeMillis());
		Ableton.getInstance().stopClip(91,1);
	}
	
	public void draw(PApplet parent, Positions p, PVector wsize) {
		final float sidesize=(wsize.x-wsize.y)/2;
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.imageMode(PConstants.CORNERS);
		//parent.image(banner, 0, 0, wsize.x, wsize.y);
		parent.colorMode(PConstants.RGB, 255);
		super.draw(parent, p, wsize);


		drawScores(parent,new PVector(sidesize,wsize.y));
		parent.translate(sidesize,0);
		drawPF(parent,p,new PVector(wsize.x-2*sidesize,wsize.y));
		parent.translate(wsize.x-2*sidesize,0);
		Clip clip=Ableton.getInstance().getClip(91, 1);
		if (clip!=null)
			drawTicker(parent,new PVector(sidesize,wsize.y),clip.position/Ableton.getInstance().getTempo()*60f);
	}

	public void drawPF(PApplet parent, Positions allpos, PVector wsize) {
		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0);
		drawBorders(parent,true,wsize);
		parent.imageMode(PConstants.CENTER);

		// Add drawing code here
		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			Position p=allpos.get(id);
			PVector offset = d.current;
			offset.sub(d.neutral);
			float angle=offset.heading();
			float dist=offset.mag();
			int quad=(int)Math.round(angle/(Math.PI/2));
			parent.pushMatrix();
			parent.translate((d.neutral.x+1)*wsize.x/2,(d.neutral.y+1)*wsize.y/2);
			parent.fill(p.getcolor(parent));
			parent.ellipse(0,0,10,10);
			parent.rotate((float)(quad*Math.PI/2+Math.PI));
			parent.translate(-ARROWDIST, 0);
			if (dist >= MINMOVEDIST)
				parent.image(arrow, 0, 0, 20, 20);
			parent.popMatrix();
		}
	}

	public void drawScores(PApplet parent, PVector wsize) {
	}

	public void drawTicker(PApplet parent, PVector wsize, float now) {
		final float DURATION=2.0f;  // Duration of display top to bottom
		final float HISTORY=0.5f;    // Amount of past showing
		ArrayList<NoteData> notes=sf.getNotes(0, now-HISTORY, now-HISTORY+DURATION);
		parent.fill(255,0,0);
		//parent.ellipse(wsize.x/2,wsize.y/2,100,100);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.textSize(16);
		parent.stroke(0,255,0);
		parent.tint(255);
		float arrowsize=wsize.x/4f/1.5f;
		for (NoteData n: notes) {
			final float angles[]={0f,(float)(Math.PI/2),(float)(3*Math.PI/2),(float)Math.PI};
			float ypos=(n.timestamp-(now-HISTORY))/DURATION*wsize.y;
			//PApplet.println("At "+n.timestamp+", notes="+n.notes+", y="+ypos);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					float xpos=(i+0.5f)*wsize.x/n.notes.length();
					//PApplet.println("x="+xpos+", text="+n.notes.substring(i,i+1));
					//parent.text(n.notes.substring(i,i+1),xpos,ypos);
					parent.pushMatrix();
					parent.translate(xpos,ypos);
					parent.rotate(angles[i]);
					parent.image(arrow, 0, 0, arrowsize, arrowsize);
					parent.popMatrix();
				}
			}
		}
		parent.line(0,HISTORY*wsize.y/DURATION,0,wsize.x,HISTORY*wsize.y/DURATION,0);
		parent.text(String.format("%.2f", now), 5, wsize.y-10);
	}
}

