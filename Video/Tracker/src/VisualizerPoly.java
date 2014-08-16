import java.util.HashMap;
import java.util.Iterator;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

// Visualizer based on iPad app "Poly" by James Milton
class PolyState {
	final float DRUMMERPROB=0.1f;
	boolean playing;
	float startBeat;
	float noteDuration;
	Person pos;
	int color;
	int mybeat;
	float lastbeat;
	int curnote;
	float lastGrouping;

	
	PolyState(Person pos, float noteDuration, int color) {
		this.noteDuration=noteDuration;
		this.color=color;
		this.pos=pos;
		playing=false;
		lastGrouping=0;
	}

	void update(float beat, int totalBeats, Scale scale, Synth synth, int channel) {
		if (playing && startBeat+noteDuration <= beat) {
			//PApplet.println("Stopped channel "+pos.channel+" at beat "+beat);
			playing=false;
		}
		float radius=pos.getNormalizedPosition().mag();
		mybeat=(int)(radius*totalBeats+0.5);
		if (mybeat==0)
			mybeat=1;
		boolean isDrummer=(channel==2);
		if (!playing && (((int)(beat*4)-(int)(startBeat*4))>=mybeat || pos.groupsize>1 ) && (int)(beat*4) != (int)(startBeat*4)) {
			if (isDrummer) {
				int pitch=(int)((pos.getNormalizedPosition().heading()+Math.PI)/(2*Math.PI)*16+35);
				synth.play(pos.id, pitch, 127, (int)(noteDuration*480), channel);
			} else {	
				// Play note
				int pitch=(int)((pos.getNormalizedPosition().heading()+Math.PI)/(2*Math.PI)*46+35);

				PApplet.println("Play note "+pitch+" on channel "+channel+" from beat "+beat+" to "+(startBeat+noteDuration));
				// Send MIDI
				synth.play(pos.id, pitch, 127, (int)(noteDuration*480), channel);
			}
			playing=true;
			startBeat=beat;
		}
		lastbeat=beat;
		if (pos.groupsize>1 && beat-lastGrouping>2) {
			isDrummer=(Math.random() <DRUMMERPROB);
			int newInstrument=(int)(Math.random()*127+1);
			PApplet.println("ID "+pos.id+" is in group ("+pos.groupid+","+pos.groupsize+"), changing to GM instrument "+newInstrument);
			OscMessage msg=new OscMessage("/midi/setpgm/"+pos.channel);
			msg.add(newInstrument);
			Tracker.sendOSC("MPO",msg);
			lastGrouping=beat;
		}
	}

	@SuppressWarnings("unused")
	void draw(PApplet parent,PVector wsize, int totalBeats, int row, Synth synth) {
		final int NUMROWS=20;
		float rowheight=wsize.y/2/NUMROWS;

		parent.stroke(color);
		if (playing)
			parent.fill(color);
		else
			parent.fill(color,127);
		float sz=Math.min(wsize.x,wsize.y);
		parent.ellipse((pos.getNormalizedPosition().x)*sz/2+wsize.x/2, (pos.getNormalizedPosition().y)*sz/2+wsize.y/2, 30, 30);
		if (playing) {
			parent.fill(0);
			parent.stroke(color);
			parent.ellipse(wsize.x/2,wsize.y/2,mybeat*sz/totalBeats,mybeat*sz/totalBeats);
		}
		if (pos.groupsize > 1) {
			final int NBOLTS=20;
			float BOLTLENGTH=wsize.y/20*pos.groupsize;
			for (int k=0;k<NBOLTS;k++)
				if (Math.random() < 0.2) {
					PVector delta=new PVector((float)Math.cos(Math.PI*2*k/NBOLTS)*BOLTLENGTH,(float)Math.sin(Math.PI*2*k/NBOLTS)*BOLTLENGTH);
					PVector center=new PVector((pos.getNormalizedPosition().x)*sz/2+wsize.x/2,(pos.getNormalizedPosition().y)*sz/2+wsize.y/2);
					parent.fill(color,127);
					parent.line(center.x,center.y,center.x+delta.x,center.y+delta.y);
				}
		}

		if (row<NUMROWS) {
			float rowpos=(row+0.5f)*rowheight;
			if (row>=NUMROWS/2)
				rowpos+=wsize.y/2;
			float dotsize=0.8f*rowheight;
			parent.fill(color);
			parent.ellipse(1+dotsize/2,rowpos, dotsize, dotsize);
			parent.fill(255);
			parent.textAlign(PConstants.LEFT,PConstants.CENTER);
			parent.textSize(0.8f*rowheight);
			if (false) {
				MidiProgram mp=synth.getMidiProgam(pos.channel);
				if (mp!=null)
					parent.text(mp.name,2+dotsize,rowpos);
			}
			}
		}
	}
	void drawLaser(PVector center, float maxRadius, int totalBeats, int row) {
		Laser laser=Laser.getInstance();
		if (playing) {
			laser.shapeBegin("circle"+mybeat);
			laser.circle(center.x, center.y, mybeat*maxRadius/totalBeats);
			laser.shapeEnd("circle"+mybeat);
		}
		if (pos.groupsize > 1) {
			final int NBOLTS=20;
			float BOLTLENGTH=(Tracker.rawmaxy-Tracker.rawminy)/20*pos.groupsize;
			for (int k=0;k<NBOLTS;k++)
				if (Math.random() < 0.2) {
					PVector delta=new PVector((float)Math.cos(Math.PI*2*k/NBOLTS)*BOLTLENGTH,(float)Math.sin(Math.PI*2*k/NBOLTS)*BOLTLENGTH);
					PVector origin=pos.getOriginInMeters();
					laser.shapeBegin("bolt"+pos.id);
					laser.line(origin.x,origin.y,origin.x+delta.x,origin.y+delta.y);
					laser.shapeEnd("bolt"+pos.id);
				}
		}
	}
}

public class VisualizerPoly extends Visualizer {
	HashMap<Integer,PolyState> poly;
	int totalBeats=16;
	float tempo=120f;
	float playTime;
	float noteDuration=0.25f ;   // in beats
	Scale scale;
	Synth synth;
	int channel;   // This is actually a song selector -- set once at start and all playing done with this channel
	
	VisualizerPoly(PApplet parent, Scale scale, Synth synth) {
		super();
		poly=new HashMap<Integer,PolyState>();
		this.scale=scale;
		this.synth=synth;
		this.channel=-1;
	}
	
	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Poly");
		Laser.getInstance().setFlag("body",1.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		this.channel=(this.channel+1)%Ableton.getInstance().trackSet.numTracks;
		PApplet.println("Poly playing on channel "+channel);
	}

	@Override
	public void stop() {
		super.stop();
	}

	@Override
	public void update(PApplet parent, People allpos) {
		Ableton.getInstance().updateMacros(allpos);

		// Update current radius of all players
		float beat=MasterClock.getBeat();
		//PApplet.println("Beat "+beat);
		for (int id: allpos.pmap.keySet()) {
			PolyState ps=poly.get(id);
			if (ps==null) {
				Person p=allpos.get(id);
				ps=new PolyState(p,noteDuration,p.getcolor(parent));
				poly.put(id, ps);
			}
			ps.update(beat,totalBeats,scale,synth,channel);
		}
		// Remove polys for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = poly.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				iter.remove();
			}
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		if (p.pmap.isEmpty())
			return;
		
		// Draw rings in gray
		parent.fill(0);
		parent.stroke(20);
		float sz=Math.min(wsize.x,wsize.y);
		for (int i=1;i<=totalBeats;i++) {
			if (i%4 == 0)
				parent.strokeWeight(2);
			else
				parent.strokeWeight(1);
			parent.ellipse(wsize.x/2,wsize.y/2,i*sz/totalBeats,i*sz/totalBeats);
		}

		// Draw each position and fired rings
		int pos=0;
		for (PolyState ps: poly.values()) {
			ps.draw(parent,wsize,totalBeats,pos,synth);
			pos++;
		}
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		PVector center=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2, (Tracker.rawminy+Tracker.rawmaxy)/2);
		float maxRadius=Math.min(Tracker.rawmaxx-Tracker.rawminx,Tracker.rawmaxy-Tracker.rawminy)/2;
		//PApplet.println("Poly drawLaser center="+center+", radius="+maxRadius);
		Laser laser=Laser.getInstance();
		laser.bgBegin();

		// Draw each position and fired rings
		int pos=0;
		for (PolyState ps: poly.values()) {
			ps.drawLaser(center,maxRadius,totalBeats,pos);
			pos++;	
		}
		laser.bgEnd();
	}

}

