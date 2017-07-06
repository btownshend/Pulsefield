package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Logger;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PShape;
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
    private final static Logger logger = Logger.getLogger(PolyState.class.getName());

	
	PolyState(Person pos, float noteDuration, int color) {
		this.noteDuration=noteDuration;
		this.color=color;
		this.pos=pos;
		playing=false;
		lastGrouping=0;
	}

	void update(float beat, int totalBeats, Scale scale, Synth synth, int channel) {
		if (playing && startBeat+noteDuration <= beat) {
			//logger.fine("Stopped channel "+pos.channel+" at beat "+beat);
			playing=false;
		}
		// Compute radius in aspect-preserved normalized coords (scaled by longest dimension to maintain aspect ratio)
		mybeat=(int)(pos.getNormalizedPosition(true).mag()*totalBeats/0.95f+0.5);
		if (mybeat==0)
			mybeat=1;
		boolean isDrummer=(channel==2);
		if (!playing && (((int)(beat*4)-(int)(startBeat*4))>=mybeat || pos.groupsize>1 ) && (int)(beat*4) != (int)(startBeat*4)) {
			if (isDrummer) {
				int pitch=(int)((pos.getNormalizedPosition(true).heading()+Math.PI)/(2*Math.PI)*16+35);
				synth.play(pos.id, pitch, 127, (int)(noteDuration*480), channel);
			} else {	
				// Play note
				int pitch=(int)((pos.getNormalizedPosition(true).heading()+Math.PI)/(2*Math.PI)*46+35);

				logger.fine("Play note "+pitch+" on channel "+channel+" from beat "+beat+" to "+(startBeat+noteDuration));
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
			logger.fine("ID "+pos.id+" is in group ("+pos.groupid+","+pos.groupsize+"), changing to GM instrument "+newInstrument);
			OscMessage msg=new OscMessage("/midi/setpgm/"+pos.channel);
			msg.add(newInstrument);
			//			Tracker.sendOSC("MPO",msg);
			lastGrouping=beat;
		}
	}

	void draw(PGraphics g,PVector center, float maxRadius, int totalBeats, int row, Synth synth) {
		final int NUMROWS=20;
		float rowheight=maxRadius/NUMROWS;

		g.stroke(color);
		if (playing)
			g.fill(color);
		else
			g.fill(color,127);

		PVector origin=pos.getOriginInMeters();
		g.ellipse(origin.x, origin.y, 0.2f, 0.2f);
		if (playing) {
			g.fill(0,0);
			g.stroke(color);
			g.ellipse(center.x,center.y,mybeat*maxRadius*2/totalBeats,mybeat*maxRadius*2/totalBeats);
		}
		if (pos.groupsize > 1) {
			final int NBOLTS=20;
			for (int k=0;k<NBOLTS;k++)
				if (Math.random() < 0.2) {
					float BOLTLENGTH=(float) (maxRadius/10*pos.groupsize*Math.random());
					PVector delta=new PVector((float)Math.cos(Math.PI*2*k/NBOLTS)*BOLTLENGTH,(float)Math.sin(Math.PI*2*k/NBOLTS)*BOLTLENGTH);
					g.fill(color,127);
					g.line(origin.x,origin.y,origin.x+delta.x,origin.y+delta.y);
				}
		}

		if (row<NUMROWS) {
			float rowpos=(row+1f)*rowheight+Tracker.miny;
			if (row>=NUMROWS/2)
				rowpos+=maxRadius;
			float dotsize=0.8f*rowheight;
			g.fill(color);
			g.ellipse(Tracker.minx+dotsize,rowpos, dotsize, dotsize);
			g.fill(255);
			g.textAlign(PConstants.LEFT,PConstants.BASELINE);
			if (true) {
				MidiProgram mp=synth.getMidiProgam(pos.channel);
				if (mp!=null)
					Visualizer.drawText(g,0.8f*rowheight,mp.name,Tracker.minx+3+dotsize,rowpos);
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
			float BOLTLENGTH=(Tracker.maxy-Tracker.miny)/20*pos.groupsize;
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
	PShape icon;
	static final String iconName="Sixteenth.svg";
	
	VisualizerPoly(PApplet parent, Scale scale, Synth synth) {
		super();
		poly=new HashMap<Integer,PolyState>();
		this.scale=scale;
		this.synth=synth;
		this.channel=-1;
		icon=parent.loadShape(Tracker.SVGDIRECTORY+iconName);
	}
	
	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Poly");
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		this.channel=(this.channel+1)%Ableton.getInstance().trackSet.numTracks;
		logger.fine("Poly playing on channel "+channel);
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
		//logger.fine("Beat "+beat);
		for (int id: allpos.pmap.keySet()) {
			PolyState ps=poly.get(id);
			if (ps==null) {
				Person p=allpos.get(id);
				ps=new PolyState(p,noteDuration,p.getcolor());
				poly.put(id, ps);
			}
			ps.update(beat,totalBeats,scale,synth,channel);
		}
		// Remove polys for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = poly.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				logger.fine("Removing ID "+id);
				iter.remove();
			}
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		
		// Draw rings in gray
		g.fill(0);
		g.stroke(20);
		float sz=Math.min(Tracker.getFloorSize().x,Tracker.getFloorSize().y)*0.95f;
		PVector center=Tracker.getFloorCenter();

		for (int i=1;i<=totalBeats;i++) {
			if (i%4 == 0)
				g.strokeWeight(0.02f);
			else
				g.strokeWeight(0.01f);
			g.ellipse(center.x,center.y,i*sz/totalBeats,i*sz/totalBeats);
		}

		// Draw each position and fired rings
		int pos=0;
		for (PolyState ps: poly.values()) {
			ps.draw(g,center,sz/2,totalBeats,pos,synth);
			pos++;
		}
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		PVector center=new PVector((Tracker.minx+Tracker.maxx)/2, (Tracker.miny+Tracker.maxy)/2);
		float maxRadius=Math.min(Tracker.maxx-Tracker.minx,Tracker.maxy-Tracker.miny)/2;
		//logger.fine("Poly drawLaser center="+center+", radius="+maxRadius);
		Laser laser=Laser.getInstance();
		laser.bgBegin();

		// Draw each position and fired rings
		int pos=0;
		for (PolyState ps: poly.values()) {
			ps.drawLaser(center,maxRadius,totalBeats,pos);
			pos++;	
		}
		laser.bgEnd();
		for (Person ps: p.pmap.values()) {  
			laser.cellBegin(ps.id);
			laser.svgfile(iconName,0.0f,0.0f,0.7f,0.0f);
			laser.cellEnd(ps.id);
		}
	}

}

