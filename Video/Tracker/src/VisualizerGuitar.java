import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;

class GString {
	final static int nfrets=22;
	static float frets[];
	final static float nut=-0.8f;  // Range of Y-coords for nut, bridge in [-1,1] normalized coordinates
	final static float bridge=0.9f;
	final static float minstring=-0.48f;  // Range of X-coords for strings in [-1,1] normalized coordinates
	final static float maxstring=0.49f;
	final static int vibrateTime=1000;   // Milliseconds
	int fretpitch;
	float position;  // X-coord of string (in range -1 to 1)
	boolean vibrating;
	long strikeTime;
	int color;
	int fret;
	int velocity;
	
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

	public void strike(Synth synth, Person p, int color) {
		int i;
		for (i=0;i<frets.length;i++)
			if (p.getNormalizedPosition().x<frets[i])
				break;
		//PApplet.println("x="+p.origin.x+", i="+i);
		if (i==0 || i>=frets.length)
			return;
		fret=i-1;
		//PApplet.println("Fret="+fret);
		vibrating=true;
		this.color=color;
		strikeTime=System.currentTimeMillis();
		int velocity=(int)(p.getVelocityInMeters().mag()*64);
		if (velocity>127)
			velocity=127;
		this.velocity=velocity;
		PApplet.println("Strike ("+p.getNormalizedPosition().x+","+p.getNormalizedPosition().y+") Vel="+velocity+", Color="+color);
		synth.play(p.id, fretpitch+fret, velocity, vibrateTime, p.channel);
	}

	public boolean isVibrating() {
		if (vibrating && System.currentTimeMillis() - strikeTime > vibrateTime )
			vibrating=false;
		return vibrating;
	}
	// Get the fraction of time elapsed in its vibration as [0,1.0];
	public float fracOfVibrate() {
		return (System.currentTimeMillis()-strikeTime)*1.0f/vibrateTime;
	}
	
	public float elapsedTime() {
		return (System.currentTimeMillis()-strikeTime)/1000f;
	}
}

public class VisualizerGuitar extends VisualizerPS {
	final GString strings[]={new GString(40),new GString(45), new GString(50),new GString(55),new GString(59),new GString(64)};
	Synth synth;
	HashMap<Integer, PVector> lastpos;
	PImage guitar;
	TrackSet trackSet;
	final static float laserScaling=0.5f;    // Scale laser drawing this much

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
		super.start();
		trackSet=Ableton.getInstance().setTrackSet("Guitar");
	}

	@Override
	public void stop() {
		super.stop();
	}


	public void draw(PApplet parent, People p, PVector wsize) {
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
				float vfrac=s.fracOfVibrate();
				PVector mid=PVector.add(PVector.mult(pf,1-vfrac),PVector.mult(p2,vfrac));
				mid.y+=s.velocity/127f*(1-vfrac)*Math.sin(2*Math.PI*s.elapsedTime()*5)*(strings[1].position-strings[0].position)*wsize.y/4;  // Max amplitude is 1/2 the spacing
				parent.stroke(s.color);
				parent.line(p1.x,p1.y,pf.x,pf.y);
				parent.line(pf.x,pf.y,mid.x,mid.y);
				parent.line(mid.x,mid.y,p2.x,p2.y);
			} else {
				parent.stroke(127);
				parent.line(p1.x,p1.y,p2.x,p2.y);
			}
		}
	}
	
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		//PApplet.println("Guitar drawLaser");
		Laser laser=Laser.getInstance();
		laser.bgBegin();
		for (int i=0;i<GString.nfrets;i++) {
			float xpos=GString.frets[i]*laserScaling;
			PVector p1=Tracker.unMapPosition(new PVector(xpos,laserScaling*(GString.minstring-.05f)));
			PVector p2=Tracker.unMapPosition(new PVector(xpos,laserScaling*(GString.maxstring+.05f)));		
			laser.line(p1.x,p1.y,p2.x,p2.y);
		}
		for (int i=0;i<strings.length;i++) {
			GString s=strings[i];
			float ypos=laserScaling*s.position;
			PVector p1=Tracker.unMapPosition(new PVector(laserScaling*GString.nut,ypos));
			PVector p2=Tracker.unMapPosition(new PVector(laserScaling*GString.bridge,ypos));	
			if (s.isVibrating()) {
				PVector pf=Tracker.unMapPosition(new PVector(GString.frets[s.fret],ypos));	
				float amp=(1-s.fracOfVibrate())*s.velocity/127.0f ;
				PVector mid=PVector.mult(PVector.add(pf,p2),0.5f);
				mid.y+=Math.sin(2*Math.PI*s.elapsedTime()*5)*laserScaling*(strings[1].position-strings[0].position)*3*amp;  
				laser.cubic(p1.x,p1.y,pf.x,pf.y,mid.x,mid.y,p2.x,p2.y);
			} else {
				laser.line(p1.x,p1.y,p2.x,p2.y);
			}
		}
		laser.bgEnd();
	}

	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
		if (lastpos != null)
			for (int id: allpos.pmap.keySet()) {
				Person p=allpos.pmap.get(id);
				PVector lastp=lastpos.get(id);
				if (lastp!=null) {
					//PApplet.println("y="+lastp.y+" -> "+p.origin.y);
					for (int i=0;i<strings.length;i++) {
						GString s=strings[i];
						if ( (p.getNormalizedPosition().y > s.position) != (lastp.y >s.position) ) {
							// Crossed a string
							s.strike(synth, p, p.getcolor(parent));
						}
					}
				}
			}
		lastpos.clear();
		for (int id: allpos.pmap.keySet()) 
			lastpos.put(id, new PVector(allpos.get(id).getNormalizedPosition().x,allpos.get(id).getNormalizedPosition().y));
	}
}
