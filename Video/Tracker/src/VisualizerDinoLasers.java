import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import DinoDuino.DinoManager;

class DinoData extends OtherPersonData {
	PVector center;  // Average position of midpoint of hands
	PVector left;    // Left extreme position (left has smaller X than right)
	int lastLeftFrame;    // Frame number last time hit left extreme
	PVector right;	// Right extreme position
	int lastRightFrame;
	PVector meanVelocity; 
	PVector movement;  // Vector of movement extremes (always oriented with a positive X value)
	int lastCrossing;
	int lastRightCrossing;
	boolean crossed;
	float phase;// Phase of motion  0 is position with lowest X value, 180 at maximum X
	float bpm;	// Beats/minute of movement
	DinoData(PVector initPos) {center=initPos; left=initPos; right=initPos; meanVelocity=new PVector(0f,0f); }
	public String toString() {
		return "center="+center+", left="+left+"("+lastLeftFrame+"), right="+right+"("+lastRightFrame+")";
	}
	public String status() {
		return String.format("BPM=%.0f Phase=%.0f",bpm,phase);
	}
};

public class VisualizerDinoLasers extends Visualizer {
	final float TC=2f;     // Time constant for average center position (in seconds)
	final float EXTREMEHOLDTIME=2.0f;  
	final float EXTREMEDECAY=1f;	// Time constant for decay of extreme positions
	final float MINCROSSINGINTERVAL=0.2f;    // Minimum time between subsequent center crossings
	final float BPMDECAY=0.25f;		// Time constant for decay of BPM estimate (per estimate)
	DinoManager dm;
	float meanBPM;
	float meanPhase;
	float meanAmplitude;
	
	VisualizerDinoLasers(PApplet parent) {
		super();
		dm=new DinoManager();
	}

	@Override
	public void start() {
		super.start();
		// Other initialization when this app becomes active
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}
	
	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
		int pcnt=0;
		meanPhase=0;
		meanAmplitude=0;
		meanBPM=0;
		
		for (Person ps: p.pmap.values()) { 
			DinoData dd=(DinoData)ps.getOther();
			if (dd==null) {
				PApplet.println("Creating new dd for person "+ps.id);
				dd=new DinoData(ps.getOriginInMeters());
				ps.setOther(dd);
			}
			PVector pos=ps.getOriginInMeters();
			dd.center=PVector.add(PVector.mult(pos,1.0f/(TC*parent.frameRate)),PVector.mult(dd.center,1-1.0f/(TC*parent.frameRate)));
			if (pos.x<dd.left.x) {
				dd.lastLeftFrame=parent.frameCount;
				dd.left=pos;
			}
			if (pos.x>dd.right.x) {
				dd.lastRightFrame=parent.frameCount;
				dd.right=pos;
			}
			if (parent.frameCount-dd.lastLeftFrame>EXTREMEHOLDTIME*parent.frameRate)
				dd.left.mult(EXTREMEDECAY);
			if (parent.frameCount-dd.lastRightFrame>EXTREMEHOLDTIME*parent.frameRate)
				dd.right.mult(EXTREMEDECAY);
			
			dd.movement=PVector.sub(dd.right,dd.left);
			if ((pos.x < dd.center.x) != dd.crossed && parent.frameCount-dd.lastCrossing > MINCROSSINGINTERVAL*parent.frameRate) {
				dd.crossed=pos.x<dd.center.x;
				int delta=parent.frameCount-dd.lastCrossing;
				float curbpm = 60f/((parent.frameCount-dd.lastCrossing)/parent.frameRate);
				dd.lastCrossing = parent.frameCount;
				dd.bpm=curbpm*BPMDECAY+dd.bpm*(1-BPMDECAY);
				if (dd.crossed)
					dd.lastRightCrossing=dd.lastCrossing;
				PApplet.println("Crossed center after "+delta+" frames, curbpm="+curbpm+", new BPM="+dd.bpm);
				PApplet.println("Person "+ps.id+": pos="+pos+", "+dd.toString());
			}
			dd.phase=(parent.frameCount-dd.lastCrossing)/parent.frameRate / (2*60/dd.bpm) * 180;
			if (dd.crossed)
				dd.phase+=180;
			if (dd.phase>360)
				dd.phase-=360;
			dm.setServo(pcnt+1, 0, (float)(Math.sin(dd.phase*Math.PI/180)*45), 1.0f/parent.frameRate);
			
			meanPhase+=dd.phase;
			meanAmplitude+=dd.movement.mag();
			meanBPM+=dd.bpm;
			pcnt++;
		}
		if (pcnt>0) {
			meanPhase/=pcnt;
			meanAmplitude/=pcnt;
			meanBPM/=pcnt;
			dm.setServo(0, 0, (float)(Math.sin(meanPhase*Math.PI/180)*45), 1.0f/parent.frameRate);
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		parent.textAlign(PConstants.LEFT, PConstants.TOP);
		parent.textSize(24);
		int pcnt=0;
		for (Person ps: p.pmap.values()) { 
			int c=ps.getcolor(parent);
			parent.fill(c,255);
			parent.stroke(c,255);
			
			DinoData dd=(DinoData)ps.getOther();
			if (dd==null)
				continue;
			float armsep=PVector.sub(ps.legs[0].getOriginInMeters(),ps.legs[1].getOriginInMeters()).mag();
			PVector sz=Tracker.mapVelocity(new PVector(armsep,armsep));
			PVector pos=Tracker.floorToNormalized(ps.getOriginInMeters());
			parent.ellipse((pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2, sz.x*wsize.x/2, sz.y*wsize.y/2);
			
			parent.text(String.format("%d %s",ps.id,dd.status()),5,5+20*pcnt);
			pcnt++;
		}	
		parent.fill(255);
		parent.text(String.format("Mean BPM=%.0f Phase=%.0f",meanBPM, meanPhase),5,5+20*pcnt);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		Laser laser=Laser.getInstance();
		laser.bgBegin();   // Start a background drawing
		for (Person ps: p.pmap.values()) {  
			laser.cellBegin(ps.id); // Start a cell-specific drawing
			float armsep=PVector.sub(ps.legs[0].getOriginInMeters(),ps.legs[1].getOriginInMeters()).mag();
			laser.svgfile("pterodactyl.svg", 0, 0, armsep*2+0.5f, 180f);
			laser.cellEnd(ps.id);
		}
		laser.bgEnd();
	}
}
