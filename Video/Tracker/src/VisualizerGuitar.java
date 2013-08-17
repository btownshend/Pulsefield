import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;
import processing.opengl.PGL;
import processing.opengl.PGraphicsOpenGL;

class GString {
	final static int nfrets=22;
	static float frets[];
	final static float nut=-0.8f;
	final static float bridge=1.1f;
	final static float minstring=-0.48f;
	final static float maxstring=0.49f;
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
		frets=new float[nfrets];
		frets[0]=nut;
		for (int i=1;i<frets.length;i++)
			frets[i]=(float) ((bridge-frets[i-1])/10+frets[i-1]);
	}

	public void setPosition(float position) {
		this.position = position;
	}

	public void strike(Synth synth, Position p, int color) {
		PApplet.println("Strike ("+p.origin.x+","+p.origin.y+") Color="+color);
		int i;
		for (i=0;i<frets.length;i++)
			if (p.origin.x<frets[i])
				break;
		//PApplet.println("x="+p.origin.x+", i="+i);
		if (i==0 || i>=frets.length)
			return;
		fret=i-1;
		//PApplet.println("Fret="+fret);
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
	final GString strings[]={new GString(40),new GString(45), new GString(50),new GString(55),new GString(59),new GString(64)};
	Synth synth;
	HashMap<Integer, PVector> lastpos;
	PImage guitar;
	TrackSet trackSet;
	
	public VisualizerGuitar(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
		lastpos=new HashMap<Integer,PVector>();
		guitar = parent.loadImage("guitar-center.png");

		for (int i=0;i<strings.length;i++)
			strings[i].setPosition(i*(GString.maxstring-GString.minstring)/(strings.length-1)+GString.minstring);
	}

	@Override
	public void start() {
		trackSet=Ableton.getInstance().setTrackSet("Guitar");
	}

	@Override
	public void stop() {
		Ableton.getInstance().setTrackSet(null);
	}


	public void draw(PApplet parent, Positions p, PVector wsize) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.ONE_MINUS_SRC_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  

		super.draw(parent,p,wsize);
		parent.tint(127);
		parent.imageMode(PConstants.CENTER);
		parent.image(guitar, wsize.x/2, wsize.y/2, wsize.x, wsize.y);
		parent.stroke(100);
		parent.strokeWeight(2);
		for (int i=0;i<GString.nfrets;i++) {
			float xpos=GString.frets[i];
			PVector p1=this.convertToScreen(new PVector(xpos,GString.minstring-.05f), wsize);
			PVector p2=this.convertToScreen(new PVector(xpos,GString.maxstring+.05f), wsize);		
			parent.line(p1.x,p1.y,p2.x,p2.y);
		}
		parent.strokeWeight(5);
		for (int i=0;i<strings.length;i++) {
			GString s=strings[i];
			float ypos=s.position;
			PVector p1=this.convertToScreen(new PVector(GString.nut,ypos), wsize);
			PVector p2=this.convertToScreen(new PVector(GString.bridge,ypos), wsize);	
			if (s.isVibrating()) {
				PVector pf=this.convertToScreen(new PVector(GString.frets[s.fret],ypos), wsize);	

				parent.stroke(s.color);
				parent.line(p1.x,p1.y,pf.x,pf.y);
				PVector mid=new PVector((p2.x+pf.x)/2,(p2.y+pf.y)/2+parent.randomGaussian()*5);
				parent.line(pf.x,pf.y,mid.x,mid.y);
				parent.line(mid.x,mid.y,p2.x,p2.y);
			} else {
				parent.stroke(127);
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
