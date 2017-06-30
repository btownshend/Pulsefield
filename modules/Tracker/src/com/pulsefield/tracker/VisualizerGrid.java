package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class VisualizerGrid extends VisualizerPS {
	HashMap<Integer,Integer> assignments;
	HashMap<Integer,String> gridColors;
	float gposx[], gposy[];
	float gridwidth, gridheight;
	int ncell;
	String songs[]={"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR","AN","PB"};
	int song=0;
	PVector titlePos = new PVector(0,0);
	
	VisualizerGrid(PApplet parent) {
		super(parent);
		assignments = new HashMap<Integer,Integer>();
		gridColors = new HashMap<Integer,String>();
		song=0;
		setupGrid();
	}
	
	public void setupGrid() {
		final float gridSpacing=0.5f;   // Grid spacing in meters
		PVector sz=Tracker.getFloorSize();
		int nrow=(int)(sz.y/gridSpacing+0.5);
		int ncol=(int)(sz.x/gridSpacing+0.5);
		ncell=nrow*ncol;
		PApplet.println("Grid has "+nrow+" rows over "+(Tracker.maxy-Tracker.miny)+" meters and "+ncol+" columns over "+(Tracker.maxx-Tracker.minx)+" meters");
		gposx=new float[ncell];
		gposy=new float[ncell];
		for (int r=0;r<nrow;r++) {
			for (int c=0;c<ncol;c++) {
				int cell=r*ncol+c;
				gposx[cell]=(2.0f*c+1)/ncol-1;
				gposy[cell]=(2.0f*r+1)/nrow-1;
			}
		}
		gridwidth=2f/ncol;
		gridheight=2f/nrow;
	}
	
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		song=(song+1)%songs.length;
		TrackSet ts=Ableton.getInstance().setTrackSet(songs[song]);
		setupGrid();
		PApplet.println("Starting grid with song "+song+": "+ts.name);
	}
	public void stop() {
		super.stop();
		assignments.clear();
	}
	
	public void songIncr(float set) {
		if (set>0)
			song=(song+1)%songs.length;
		PApplet.println("Song="+song);
		TrackSet ts=Ableton.getInstance().setTrackSet(songs[song]);
		PApplet.println("Starting grid with song "+song+": "+ts.name);
	}


	public void update(PApplet parent, People allpos) {
		if (allpos.pmap.size() == 0)
			songIncr(1);
		super.update(parent,allpos);
		titlePos.x=Tracker.minx+Tracker.getFloorSize().x/4;
		titlePos.y=Tracker.miny+0.24f+0.1f;
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Person pos: allpos.pmap.values()) {
			//PApplet.println("ID "+pos.id+" pos="+pos.origin);
			// Find closest grid position
			// Check for song advance
			if (PVector.sub(titlePos, pos.getOriginInMeters()).mag() < 0.3f) {
				// Change song
				songIncr(1);
			}
			int closest=-1;
			int current=-1;
			double mindist=1e10f;
			// Check if we already had one
			if (assignments.containsKey(pos.id)) {
				current=assignments.get(pos.id);
				closest=current;
				mindist=(Math.pow(gposx[closest]-pos.getNormalizedPosition().x,2)+Math.pow(gposy[closest]-pos.getNormalizedPosition().y,2))*0.8;  // Make it appear a little closer to create hysteresis
				//PApplet.println("Had existing grid "+closest+" at distance "+Math.sqrt(mindist));	
			}
			for (int i=0;i<ncell;i++) {
				double dist2=Math.pow(gposx[i]-pos.getNormalizedPosition().x,2)+Math.pow(gposy[i]-pos.getNormalizedPosition().y,2);
				if (dist2 < mindist) {
					mindist=dist2;
					closest=i;
				}
			}
			//PApplet.println("ID "+pos.id+" closest to grid ("+gposx[closest]+","+gposy[closest]+"), position "+closest+" at a distance of "+Math.sqrt(mindist));
			if (current!=closest) {
				TrackSet ts=Ableton.getInstance().trackSet;
				int track=pos.id%(ts.numTracks)+ts.firstTrack;
//				if (current!=-1)
//					Ableton.getInstance().stopClip(track, current);
				assignments.put(pos.id,closest);
				int nclips=Ableton.getInstance().getTrack(track).numClips();
				if (nclips!=-1)
					Ableton.getInstance().playClip(track,closest%nclips);
			}
		}

		Iterator<Map.Entry<Integer,Integer> > i = assignments.entrySet().iterator();
		while (i.hasNext()) {
			int id=i.next().getKey();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("update: no update info for assignment for id "+id);
				i.remove();
			}
		}
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		PVector gridOffset=new PVector(gridwidth/2, gridheight/2);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id=entry.getKey();
			int cell=entry.getValue();
			//PApplet.println("grid "+cell+", id="+id+" "+gridColors.get(cell));
			g.fill(127,0,0,127);
			g.strokeWeight(.05f);
			g.stroke(255,0,0);
			PVector gcenter=new PVector(gposx[cell],gposy[cell]);
			PVector tl = Tracker.normalizedToFloor(PVector.sub(gcenter, gridOffset));
			PVector br = Tracker.normalizedToFloor(PVector.add(gcenter, gridOffset));
			g.rect(tl.x,tl.y,(br.x-tl.x),(br.y-tl.y));
			g.fill(255);
			TrackSet ts=Ableton.getInstance().trackSet;
			Track track=Ableton.getInstance().getTrack(id%(ts.numTracks)+ts.firstTrack);
			if (track.numClips()>0) {
				Clip clip=track.getClip(cell%track.numClips());
				drawText(g,0.16f,track.getName()+"-"+clip.getName()+" P"+id,gcenter.x,gcenter.y,gridOffset.x,gridOffset.y);
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
		PVector gridOffset=new PVector(gridwidth/2, gridheight/2);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int cell=entry.getValue();
			PVector gcenter=new PVector(gposx[cell],gposy[cell]);
			PVector tl = Tracker.normalizedToFloor(PVector.sub(gcenter, gridOffset));
			PVector br = Tracker.normalizedToFloor(PVector.add(gcenter, gridOffset));
			//PApplet.println("Drawing rect "+tl+" to "+br);
			laser.shapeBegin("gridcell"+cell);
			laser.rect(tl.x,tl.y,(br.x-tl.x),(br.y-tl.y));
			laser.shapeEnd("gridcell"+cell);
		}
		laser.bgEnd();
	}
	
	public void setnpeople(int n) {
		// Ignored for now
	}

	public void handleMessage(OscMessage theOscMessage) {
		//PApplet.println("Ableton message: "+theOscMessage.toString());
		boolean handled=false;
		String pattern=theOscMessage.addrPattern();
		String components[]=pattern.split("/");

		if (components[1].equals("grid") && components[2].equals("cell")) {
			int cell=Integer.parseInt(components[3])-1;  // Cell is 1-60; change to 0
			if(cell<0 || cell>=60) {
				PApplet.println("Bad grid cell: "+cell);
				cell=0;
			}
			if (components.length == 4) {
				String value=theOscMessage.get(0).stringValue();
				//PApplet.println("cell "+cell+" = "+value);
				if (value.isEmpty()) {
					assignments.remove(cell);
					gridColors.remove(cell);
				} else
					assignments.put(cell,Integer.parseInt(value));
				handled=true;
			} else if (components.length ==5 && components[4].equals("color")) {
				String color=theOscMessage.get(0).stringValue();
				gridColors.put(cell,color);
				handled=true;
			}
		} else if (components[1].equals("grid") && components[2].equals("song")) {
			Ableton.getInstance().setTrackSet(theOscMessage.get(0).stringValue());
			handled=true;
		}
		if (!handled)
			super.handleMessage(theOscMessage);
	}
}

