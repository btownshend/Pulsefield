import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

public class VisualizerGrid extends VisualizerPS {
	HashMap<Integer,Integer> assignments;
	HashMap<Integer,String> gridColors;
	float gposx[], gposy[];
	float gridwidth, gridheight;
	int ncell;
	String songs[]={"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR"};
	int song=0;
	
	VisualizerGrid(PApplet parent) {
		super(parent);
		assignments = new HashMap<Integer,Integer>();
		gridColors = new HashMap<Integer,String>();
		song=0;
		setupGrid();
	}
	
	public void setupGrid() {
		final float gridSpacing=0.5f;   // Grid spacing in meters
		int nrow=(int)((Tracker.maxy-Tracker.miny)/gridSpacing+0.5);
		int ncol=(int)((Tracker.maxx-Tracker.minx)/gridSpacing+0.5);
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

	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Person pos: allpos.pmap.values()) {
			//PApplet.println("ID "+pos.id+" pos="+pos.origin);
			// Find closest grid position
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

	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent,p, wsize);

		parent.textSize(16);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id=entry.getKey();
			int cell=entry.getValue();
			//PApplet.println("grid "+cell+", id="+id+" "+gridColors.get(cell));
			parent.fill(127,0,0,127);
			parent.strokeWeight(5);
			parent.stroke(127,0,0);
			parent.rect(wsize.x*(gposx[cell]-gridwidth/2+1)/2,wsize.y*(gposy[cell]-gridheight/2+1)/2,wsize.x*gridwidth/2,wsize.y*gridheight/2);
			parent.fill(255);
			TrackSet ts=Ableton.getInstance().trackSet;
			Track track=Ableton.getInstance().getTrack(id%(ts.numTracks)+ts.firstTrack);
			Clip clip=track.getClip(cell%track.numClips());
			
			parent.text(track.getName()+"-"+clip.getName()+" P"+id,wsize.x*(gposx[cell]-gridwidth/2+1)/2,wsize.y*(gposy[cell]-gridheight/2+1)/2,wsize.x*gridwidth/2,wsize.y*gridheight/2);
		}
		parent.fill(127);
		parent.textAlign(PConstants.LEFT, PConstants.TOP);
		parent.textSize(24);
		parent.text(Ableton.getInstance().trackSet.name,5,5);
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

