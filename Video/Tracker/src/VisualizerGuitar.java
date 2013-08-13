import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PVector;

class GString {
	final static int nfrets=10;
	final static float minfret=-0.8f;
	final static float maxfret=0.8f;
	final static float minstring=-0.6f;
	final static float maxstring=0.6f;
	final static int vibrateTime=500;   // Milliseconds

	int fretpitch;
	float position;  // X-coord of string (in range -1 to 1)
	boolean vibrating;
	long strikeTime;
	int color;
	int fret;
	
	GString(int fretpitch) {
		this.fretpitch=fretpitch;
		vibrating=false;
	}

	public void setPosition(float position) {
		this.position = position;
	}

	public void strike(Synth synth, Position p, int color) {
		PApplet.println("Strike ("+p.origin.x+","+p.origin.y+") Color="+color);
		fret=(int)((p.origin.x-minfret)/(maxfret-minfret)*(nfrets-1));
		if (fret < 0 || fret>= nfrets)
			return;
		PApplet.println("Fret="+fret);
		vibrating=true;
		this.color=color;
		strikeTime=System.currentTimeMillis();
		int velocity=(int)(p.avgspeed.mag()*500);
		if (velocity>127)
			velocity=127;
		synth.play(p.id, fretpitch+fret, velocity, vibrateTime, p.channel);
	}

	public boolean isVibrating() {
		if (vibrating && System.currentTimeMillis() - strikeTime > vibrateTime )
			vibrating=false;
		return vibrating;
	}

}

public class VisualizerGuitar extends VisualizerPS {
	final GString strings[]={new GString(64),new GString(59), new GString(55),new GString(50),new GString(45),new GString(40)};
	Synth synth;
	HashMap<Integer, PVector> lastpos;

	public VisualizerGuitar(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
		lastpos=new HashMap<Integer,PVector>();
		
		for (int i=0;i<strings.length;i++)
			strings[i].setPosition(i*(GString.maxstring-GString.minstring)/(strings.length-1)+GString.minstring);
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		super.draw(parent,p,wsize);
		parent.stroke(255);
		parent.strokeWeight(5);
		for (int i=0;i<GString.nfrets;i++) {
			float xpos=i*(GString.maxfret-GString.minfret)/(GString.nfrets-1)+GString.minfret;
			PVector p1=this.convertToScreen(new PVector(xpos,GString.minstring), wsize);
			PVector p2=this.convertToScreen(new PVector(xpos,GString.maxstring), wsize);		
			parent.line(p1.x,p1.y,p2.x,p2.y);
		}
		parent.strokeWeight(2);
		for (int i=0;i<strings.length;i++) {
			GString s=strings[i];
			float ypos=s.position;
			PVector p1=this.convertToScreen(new PVector(GString.minfret,ypos), wsize);
			PVector p2=this.convertToScreen(new PVector(GString.maxfret,ypos), wsize);	
			PVector pf=this.convertToScreen(new PVector(s.fret*(GString.maxfret-GString.minfret)/(GString.nfrets-1)+GString.minfret,ypos), wsize);	
			if (s.isVibrating()) {
				parent.stroke(s.color);
				parent.line(p1.x,p1.y,pf.x,pf.y);
				PVector mid=new PVector((p2.x+pf.x)/2,(p2.y+pf.y)/2+parent.randomGaussian()*5);
				parent.line(pf.x,pf.y,mid.x,mid.y);
				parent.line(mid.x,mid.y,p2.x,p2.y);
			} else {
				parent.stroke(255);
				parent.line(p1.x,p1.y,p2.x,p2.y);
			}
		}
	}

	public void update(PApplet parent, Positions allpos) {
		super.update(parent,allpos);
		if (lastpos != null)
			for (int id: allpos.positions.keySet()) {
				Position p=allpos.positions.get(id);
				PVector lastp=lastpos.get(id);
				if (lastp!=null) {
					//PApplet.println("y="+lastp.y+" -> "+p.origin.y);
					for (int i=0;i<strings.length;i++) {
						GString s=strings[i];
						if ( (p.origin.y > s.position) != (lastp.y >s.position) ) {
							// Crossed a string
							s.strike(synth, p, p.getcolor(parent));
						}
					}
				}
			}
		lastpos.clear();
		for (int id: allpos.positions.keySet()) 
			lastpos.put(id, new PVector(allpos.get(id).origin.x,allpos.get(id).origin.y));
	}
}
