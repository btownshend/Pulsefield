import java.util.ArrayList;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;
class NoteSpot {
	int pitch;
	int velocity; // 0-127
	int duration;  // in msec
	PVector pos;
	float radius;
	int activeFrames;  // number of frames that this note will still be active
	
	NoteSpot(int pitch, int velocity, int duration, PVector pos, float radius) {
		this.pitch=pitch;
		this.velocity=velocity;
		this.duration=duration;
		this.pos=pos;
		this.radius=radius;
	}
	
	boolean hitTest(PVector tgt) {
		return PVector.sub(pos, tgt).mag()<=radius;
	}
	
	void setActive(int frames) {
		activeFrames=frames;
	}
}

public class VisualizerNotes extends VisualizerDot {
	ArrayList<NoteSpot> notes;
	Synth synth;

	VisualizerNotes(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
		notes=new ArrayList<NoteSpot>();
		notes.add(new NoteSpot(60,127,1000,new PVector(-1f,2.5f),0.2f));
		notes.add(new NoteSpot(62,127,1000,new PVector(1f,2.5f),0.2f));
	}

	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("SteelPan");
	}

	@Override
	public void stop() {
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		int oldPitch[]=new int[2];
		int newPitch[]=new int[2];
		
		for (Person ps: p.pmap.values()) {  
			oldPitch[0]=((int)ps.userData)%1000;
			oldPitch[1]=((int)ps.userData)/1000;
			newPitch[0]=0;
			newPitch[1]=0;
			for (NoteSpot noteSpot: notes) {	
				for (int i=0;i<ps.legs.length;i++) {
					Leg leg=ps.legs[i];
					if (noteSpot.hitTest(leg.getOriginInMeters())) {
						newPitch[i]=noteSpot.pitch;
						if (oldPitch[i]!=noteSpot.pitch) {
							// New pitch, trigger it
							synth.play(ps.id, noteSpot.pitch, noteSpot.velocity, noteSpot.duration, ps.channel);
						}
					}
				}
			}
			ps.userData=newPitch[0]+newPitch[1]*1000;
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		g.ellipseMode(PConstants.CENTER);
		g.fill(0,255,0);
		g.stroke(255,0,0);
		for (NoteSpot noteSpot: notes) {	
			g.ellipse(noteSpot.pos.x, noteSpot.pos.y, noteSpot.radius*2, noteSpot.radius*2);
		}
	}
}
