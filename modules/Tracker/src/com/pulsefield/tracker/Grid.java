package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.logging.Logger;

import oscP5.OscMessage;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class Grid {
    protected final static Logger logger = Logger.getLogger(Grid.class.getName());
    ParticleSystem universe;
	String oscName = "particlefield";

    
	HashMap<Integer,Integer> assignments;
	HashMap<Integer,String> gridColors;
	float gposx[], gposy[];
	float gridwidth, gridheight;
	int ncell;
	String songs[];
	int song=0;
	PVector titlePos = new PVector(0,0);
	private boolean didIncr = false;
	
	Grid(String songs[]) {
		this.songs=songs;
		assignments = new HashMap<Integer,Integer>();
		gridColors = new HashMap<Integer,String>();
		song=0;
		setupGrid();
		universe=new ParticleSystem();
	}
	
	public void setupGrid() {
		final float gridSpacing=0.5f;   // Grid spacing in meters
		PVector sz=Tracker.getFloorSize();
		int nrow=(int)(sz.y/gridSpacing+0.5);
		int ncol=(int)(sz.x/gridSpacing+0.5);
		ncell=nrow*ncol;
		logger.info("Grid has "+nrow+" rows over "+(Tracker.maxy-Tracker.miny)+" meters and "+ncol+" columns over "+(Tracker.maxx-Tracker.minx)+" meters");
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
		song=(song+1)%songs.length;
		TrackSet ts=Ableton.getInstance().setTrackSet(songs[song]);
		setupGrid();
		logger.info("Starting grid with song "+song+": "+ts.name);
	}
	
	public void stop() {
		assignments.clear();
	}
	
	public void songIncr(float set) {
		if (set>0)
			song=(song+1)%songs.length;
		logger.fine("Song="+song);
		TrackSet ts=Ableton.getInstance().setTrackSet(songs[song]);
		logger.info("Starting grid with song "+song+": "+ts.name);
	}


	public void update(People allpos) {
		if (allpos.pmap.size() == 0) {
			if (!didIncr)
				songIncr(1);
			didIncr=true;
		} else
			didIncr=true;
		titlePos.x=Tracker.minx+Tracker.getFloorSize().x/4;
		titlePos.y=Tracker.miny+0.24f+0.1f;
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Person pos: allpos.pmap.values()) {
			//logger.fine("ID "+pos.id+" pos="+pos.origin);
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
				//logger.fine("Had existing grid "+closest+" at distance "+Math.sqrt(mindist));	
			}
			for (int i=0;i<ncell;i++) {
				double dist2=Math.pow(gposx[i]-pos.getNormalizedPosition().x,2)+Math.pow(gposy[i]-pos.getNormalizedPosition().y,2);
				if (dist2 < mindist) {
					mindist=dist2;
					closest=i;
				}
			}
			//logger.fine("ID "+pos.id+" closest to grid ("+gposx[closest]+","+gposy[closest]+"), position "+closest+" at a distance of "+Math.sqrt(mindist));
			if (current!=closest) {
				TrackSet ts=Ableton.getInstance().trackSet;
				int track=pos.id%(ts.numTracks)+ts.firstTrack;
//				if (current!=-1)
//					Ableton.getInstance().stopClip(track, current);
				assignments.put(pos.id,closest);
				int nclips=Ableton.getInstance().getTrack(track).numClips();
				if (nclips>0)
					Ableton.getInstance().playClip(track,closest%nclips);
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

	public void draw(PGraphics g, People p) {
		if (p.pmap.isEmpty())
			return;
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		PVector gridOffset=new PVector(gridwidth/2, gridheight/2);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id=entry.getKey();
			int cell=entry.getValue();
			//logger.fine("grid "+cell+", id="+id+" "+gridColors.get(cell));

			TrackSet ts=Ableton.getInstance().trackSet;
			Track track=Ableton.getInstance().getTrack(id%(ts.numTracks)+ts.firstTrack);
			Clip clip=track.getClip(cell%track.numClips());

			float lev=Math.max(track.meter[0],track.meter[1]);   // Level should go from 1.0->0.0
			//float pos=Math.min(1.0f,clip.position);   // Track time scaled down to approx 0.0-1.0
			
			g.fill(127,0,0,127*lev);
			g.strokeWeight(.05f*lev);
			g.stroke(255,0,0,255*lev);
			PVector gcenter=new PVector(gposx[cell],gposy[cell]);
			PVector tl = Tracker.normalizedToFloor(PVector.sub(gcenter, PVector.mult(gridOffset,lev)));
			PVector br = Tracker.normalizedToFloor(PVector.add(gcenter, PVector.mult(gridOffset,lev)));
			if (lev>0) {
				g.rect(tl.x,tl.y,(br.x-tl.x),(br.y-tl.y));
				for (int i=0;i<universe.settings.genRate/Tracker.theTracker.frameRate;i++) {
					Particle particle=new Particle(Tracker.normalizedToFloor(gcenter),universe.settings);
					particle.color=p.get(id).getcolor();
					float theta=(float)(Math.random()*2*Math.PI);
					//final float spinPeriod=1.0f;
					//float theta=(float)((clip.position+i/20.0/10+Math.random()*.1) * Math.PI*2/spinPeriod);
					float speed=0.05f/clip.length;
					particle.velocity=(new PVector(lev*(float)(speed+speed/10*Math.random()),0)).rotate(theta);
					particle.opacity=lev*lev;
//					final float accel=(float)(0.0002f*Math.random())*0;
//					if (track.meter[0]>track.meter[1])
//						particle.acceleration=new PVector(accel,0);
//					else
//						particle.acceleration=new PVector(-accel,0);

					universe.addParticle(particle);
				}
			}
			g.fill(255);

			if (track.numClips()>0) {
				String t=String.format("%s-%s[%d] %.2f L=%f,R=%f, ID=%d", track.getName(),clip.getName(),clip.clipNum,clip.position,track.meter[0],track.meter[1],id);
				//logger.info("At "+tl+"; "+br+": "+t);
				Visualizer.drawText(g,0.08f,t,br.x,tl.y,br.x-tl.x,br.y-tl.y);
			}
		}
		universe.update();
		universe.draw(g);
	}

	public void drawTitle(PGraphics g) {
		g.fill(127);
		g.textAlign(PConstants.LEFT, PConstants.BASELINE);
		Visualizer.drawText(g,0.24f,Ableton.getInstance().trackSet.name,titlePos.x,titlePos.y);
	}
	
	public boolean handleMessage(OscMessage theOscMessage) {
		//logger.fine("Ableton message: "+theOscMessage.toString());
		boolean handled=false;
		String pattern=theOscMessage.addrPattern();
		String components[]=pattern.split("/");

		if (components[1].equals("grid") && components[2].equals("cell")) {
			int cell=Integer.parseInt(components[3])-1;  // Cell is 1-60; change to 0
			if(cell<0 || cell>=60) {
				logger.warning("Bad grid cell: "+cell);
				cell=0;
			}
			if (components.length == 4) {
				String value=theOscMessage.get(0).stringValue();
				//logger.fine("cell "+cell+" = "+value);
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
		} else if (theOscMessage.addrPattern().startsWith("/video/particlefield")) {
			handleParticleMessage(theOscMessage);
			handled=true;
		}
		return handled;
	}
	
	public void handleParticleMessage(OscMessage msg) {
		if (universe == null)
			universe = new ParticleSystem();

		logger.fine("Grid Particle message: " + msg.toString());

		String pattern = msg.addrPattern();
		String components[] = pattern.split("/");
		PApplet.println("ParticleField OSC Message (" + components.length + " len) " + msg.toString() + " : "
				+ msg.get(0).floatValue());

		if (components.length < 3 || !components[2].equals(oscName))
			logger.warning("VisualizerParticleSystem: Expected /video/" + oscName + " messages, got " + msg.toString());
		else if (components.length == 4 && components[3].equals("maxparticles")) {
			universe.settings.maxParticles = (long) Math.pow(2, msg.get(0).floatValue());
		} else if (components.length == 4 && components[3].equals("particledispersion")) {
			universe.settings.particleRandomDriftAccel = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("forcerotation")) {
			universe.settings.forceRotation = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlerotation")) {
			universe.settings.particleRotation = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("genrate")) {
			universe.settings.genRate = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlemaxlife")) {
			universe.settings.particleMaxLife = (int) msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particleopacity")) {
			universe.settings.startOpacity = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("persongravity")) {
			universe.settings.personForce = msg.get(0).floatValue();
		} else if (components.length == 4 && components[3].equals("particlescale")) {
			universe.settings.particleScale = msg.get(0).floatValue();
		} else if (components.length == 6 && components[3].equals("blendmode")) {
			handleBlendSettingMessage(msg);
		} else if (components.length == 4 && components[3].equals("tilt")) {
			handleTilt(msg);
		} else
			logger.warning("unhandled OSC message"+msg);

		setTO();
	}
	

	private void handleTilt(OscMessage msg) {
		msg.addrPattern().split("/");

		universe.settings.tiltx = msg.get(0).floatValue() * 0.0001f;
		universe.settings.tilty = msg.get(1).floatValue() * 0.0001f;
	}

	private void handleBlendSettingMessage(OscMessage msg) {
		String components[] = msg.addrPattern().split("/");

		if (components.length < 5) {
			logger.warning("handleBlendSettingMessage: Expected more components in " + msg.toString());
		} else {
			// If it's an "off" (0.0) message, ignore it.
			if (msg.get(0).floatValue() == 0.0) {
				return;
			}
			String clicked = components[4].toString() + "/" + components[5].toString();

			switch (clicked) {
			case "2/1":
				universe.settings.blendMode = -1; // Custom.
				break;
			case "2/2":
				universe.settings.blendMode = PConstants.ADD;
				break;
			case "2/3":
				universe.settings.blendMode = PConstants.SUBTRACT;
				break;
			case "2/4":
				universe.settings.blendMode = PConstants.SCREEN;
				break;
			case "1/1":
				universe.settings.blendMode = PConstants.LIGHTEST;
				break;
			case "1/2":
				universe.settings.blendMode = PConstants.BLEND;
				break;
			case "1/3":
				universe.settings.blendMode = PConstants.OVERLAY;
				break;
			case "1/4":
				universe.settings.blendMode = PConstants.MULTIPLY;
				break;
			}
		}
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to = TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/" + oscName + "/" + name);
		set.add(value);
		to.sendMessage(set);
		set = new OscMessage("/video/" + oscName + "/" + name + "/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}

	public void setTO() {
		setTOValue("maxparticles", Math.log(universe.settings.maxParticles) / Math.log(2), "%.4f");
		setTOValue("particleaccel", universe.settings.particleRandomDriftAccel, "%.4f");
		setTOValue("forcerotation", universe.settings.forceRotation, "%.4f");
		setTOValue("particlerotation", universe.settings.particleRotation, "%.4f");
		setTOValue("particlemaxlife", universe.settings.particleMaxLife, "%.4f");
		setTOValue("particleopacity", universe.settings.startOpacity, "%.4f");
		setTOValue("persongravity", universe.settings.personForce, "%.4f");
		setTOValue("particlescale", universe.settings.particleScale, "%.4f");
		setTOValue("genrate", universe.settings.genRate, "%.4f");
		setTiltTO();
	}

	public void setTiltTO() {
		TouchOSC to = TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/" + oscName + "/tilt");
		set.add("set");
		set.add(universe.settings.tiltx);
		set.add(universe.settings.tilty);
		to.sendMessage(set);
	}
}

