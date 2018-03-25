package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class VisualizerProximity extends VisualizerPS {
	HashMap<Integer,Integer> assignments;
	String songs[]={"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR","PB"};
	int song=0;
	TrackSet ts;
	static final float MAXSEP=0.2f; // Maximum separation to trigger
	PVector titlePos = new PVector(0,0);;
	
	VisualizerProximity(PApplet parent) {
		super(parent);
		assignments = new HashMap<Integer,Integer>();
		song=0;
		ts=Ableton.getInstance().setTrackSet(songs[song]);
	}
	
	public void start() {
		super.start();
		song=(song+1)%songs.length;
		ts=Ableton.getInstance().setTrackSet(songs[song]);
		logger.info("Starting proximity with song "+song+": "+ts.name);
		Laser.getInstance().setFlag("body",1.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}
	
	public void stop() {
		super.stop();
		assignments.clear();
	}

	public void songIncr(float set) {
		if (set>0)
			song=(song+1)%songs.length;
		logger.fine("Song="+song);
		TrackSet ts=Ableton.getInstance().setTrackSet(songs[song]);
		logger.fine("Starting grid with song "+song+": "+ts.name);
	}

	public int clipNumber(int nclips,int id1, int id2) {
		if (nclips==0) {
			logger.severe("nclips=0");
			return 0;
		}
		return (id1*7+id2)%nclips;
	}
	
	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
		ts=Ableton.getInstance().trackSet;
		titlePos.x=Tracker.minx+Tracker.getFloorSize().x/4;
		titlePos.y=Tracker.miny+0.24f+0.1f;
		
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Map.Entry<Integer,Person> e1: allpos.pmap.entrySet()) {
			int id1=e1.getKey();
			// Check for song advance
			if (PVector.sub(titlePos, e1.getValue().getOriginInMeters()).mag() < 0.3f) {
				// Change song
				songIncr(1);
			}
			PVector pos1=e1.getValue().getNormalizedPosition();
			// Find closest NEIGHBOR	
			int closest=-1;
			double mindist=MAXSEP;
			int current=-1;
			if (assignments.containsKey(id1))
				current=assignments.get(id1);
			
			for (Map.Entry<Integer,Person> e2: allpos.pmap.entrySet()) {
				int id2=e2.getKey();
				if (id1==id2)
					continue;
				PVector pos2=e2.getValue().getNormalizedPosition();
				
				double dist2=Math.pow(pos1.x-pos2.x,2)+Math.pow(pos1.y-pos2.y,2);
				if (id2==current)
					dist2*=0.9;  // Hysteresis
				
				if (dist2 < mindist) {
					mindist=dist2;
					closest=id2;
				}
			}
			//logger.fine("ID "+id1+" closest to id "+closest+" at a distance of "+Math.sqrt(mindist));

			if (current!=closest) {
				logger.fine("ID "+closest+" now closer to id "+id1+" instead of "+current+" at a distance of "+Math.sqrt(mindist));

				int track=id1%(ts.numTracks)+ts.firstTrack;
//				if (current!=-1)
//					Ableton.getInstance().stopClip(track, clipNumber(id1,closest));
				assignments.put(id1,closest);
				if (closest!=-1) {
					int nclips=Ableton.getInstance().getTrack(track).numClips();
					if (nclips!=-1)
						Ableton.getInstance().playClip(track,clipNumber(nclips,id1,closest));
				}
			}
		}

		Iterator<Map.Entry<Integer,Integer> > i = assignments.entrySet().iterator();
		while (i.hasNext()) {
			int id=i.next().getKey();
			if (!allpos.pmap.containsKey(id)) {
				logger.fine("update: no update info for assignment for id "+id);
				i.remove();
			}
		}
	}

	private void randomline(PGraphics g, PVector p1, PVector p2, float rfrac) {
		float step=0.02f;
		float dist=PVector.sub(p1, p2).mag();
		if (dist < step)
			g.line(p1.x, p1.y, p2.x, p2.y);
		else {
			PVector mid=PVector.mult(PVector.add(p1, p2), 0.5f);
			mid.x+=(Math.random()*2-1)*dist*rfrac;
			mid.y+=(Math.random()*2-1)*dist*rfrac;
			randomline(g,p1,mid,rfrac);
			randomline(g,mid,p2,rfrac);
		}
	}
	
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id1=entry.getKey();
			int id2=entry.getValue();
			if (id2!=-1) {
				//logger.fine("grid "+cell+", id="+id+" "+gridColors.get(cell));
				g.pushStyle();
				g.fill(127,0,0,127);
				g.strokeWeight(0.01f);
				g.colorMode(PConstants.HSB);
				int color=(id1*34813747+id2*23873)&0xff;
				//logger.fine("id1="+id1+", id2="+id2+" -> color="+color);
				g.stroke(color,255,255);
				randomline(g, p.get(id1).legs[0].getOriginInMeters(), p.get(id2).legs[0].getOriginInMeters(),.2f);
				randomline(g, p.get(id1).legs[0].getOriginInMeters(), p.get(id2).legs[1].getOriginInMeters(),.2f);
				randomline(g, p.get(id1).legs[1].getOriginInMeters(), p.get(id2).legs[0].getOriginInMeters(),.2f);
				randomline(g, p.get(id1).legs[1].getOriginInMeters(), p.get(id2).legs[1].getOriginInMeters(),.2f);
				g.popStyle();
			}
		}

		g.fill(127);
		g.textAlign(PConstants.LEFT, PConstants.BASELINE);
		drawText(g,0.24f,Ableton.getInstance().trackSet.name,titlePos.x,titlePos.y);
	}

	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		Laser laser=Laser.getInstance();
		laser.bgBegin();
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id1=entry.getKey();
			int id2=entry.getValue();
			if (id2==-1)
				continue;
			laser.shapeBegin("prox:"+id1+"-"+id2);
			PVector p1 = Tracker.normalizedToFloor(p.get(id1).getNormalizedPosition());
			PVector p2 = Tracker.normalizedToFloor(p.get(id2).getNormalizedPosition());
//			logger.fine("Drawing line "+p1+" to "+p2);
			laser.line(p1.x,p1.y,p2.x,p2.y);
			laser.shapeEnd("prox:"+id1+"-"+id2);
		}
		laser.bgEnd();
	}
	
	public void setnpeople(int n) {
		// Ignored for now
	}
}

