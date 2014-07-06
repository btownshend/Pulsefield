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
	TrackSet ts;
	
	VisualizerGrid(PApplet parent) {
		super(parent);
		assignments = new HashMap<Integer,Integer>();
		gridColors = new HashMap<Integer,String>();
		int nrow=8;
		int ncol=8;
		ncell=nrow*ncol;
		
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
		song=0;
		ts=Ableton.getInstance().setTrackSet(songs[song]);
	}
	public void start() {
		song=(song+1)%songs.length;
		ts=Ableton.getInstance().setTrackSet(songs[song]);
		PApplet.println("Starting grid with song "+song+": "+ts.name);
	}
	public void stop() {
		assignments.clear();
		}

	public void update(PApplet parent, Positions allpos) {
		super.update(parent,allpos);
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Position pos: allpos.positions.values()) {
			//PApplet.println("ID "+pos.id+" pos="+pos.origin);
			// Find closest grid position
			int closest=-1;
			int current=-1;
			double mindist=1e10f;
			// Check if we already had one
			if (assignments.containsKey(pos.id)) {
				current=assignments.get(pos.id);
				closest=current;
				mindist=(Math.pow(gposx[closest]-pos.origin.x,2)+Math.pow(gposy[closest]-pos.origin.y,2))*0.5*0.5;  // Make it appear a little closer to create hysteresis
				//PApplet.println("Had existing grid "+closest+" at distance "+Math.sqrt(mindist));	
			}
			for (int i=0;i<ncell;i++) {
				double dist2=Math.pow(gposx[i]-pos.origin.x,2)+Math.pow(gposy[i]-pos.origin.y,2);
				if (dist2 < mindist) {
					mindist=dist2;
					closest=i;
				}
			}
			//PApplet.println("ID "+pos.id+" closest to grid ("+gposx[closest]+","+gposy[closest]+"), position "+closest+" at a distance of "+Math.sqrt(mindist));
			if (current!=closest) {
				int track=pos.id%(ts.numTracks)+ts.firstTrack;
				if (current!=-1)
					Ableton.getInstance().stopClip(track, current);
				assignments.put(pos.id,closest);
				Ableton.getInstance().playClip(track,closest);
			}
		}

		Iterator<Map.Entry<Integer,Integer> > i = assignments.entrySet().iterator();
		while (i.hasNext()) {
			int id=i.next().getKey();
			if (!allpos.positions.containsKey(id)) {
				PApplet.println("update: no update info for assignment for id "+id);
				i.remove();
			}
		}
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		super.draw(parent,p, wsize);
		parent.fill(0);
		parent.stroke(255);
		parent.strokeWeight(1);
		super.drawBorders(parent, true, wsize);

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
			parent.text("C"+cell+"-P"+id,wsize.x*(gposx[cell]-gridwidth/2+1)/2,wsize.y*(gposy[cell]-gridheight/2+1)/2,wsize.x*gridwidth/2,wsize.y*gridheight/2);
		}
		parent.fill(127);
		parent.textAlign(PConstants.LEFT, PConstants.TOP);
		parent.textSize(24);
		parent.text(ts.name,5,5);
	}

	public void drawLaser(PApplet parent, Positions p) {
		super.drawLaser(parent,p);
		Laser laser=Laser.getInstance();
		PVector gridOffset=new PVector(gridwidth/2, gridheight/2);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id=entry.getKey();
			int cell=entry.getValue();
			laser.cellBegin(id);
			PVector gcenter=new PVector(gposx[cell],gposy[cell]);
			PVector tl = Tracker.unMapPosition(PVector.sub(gcenter, gridOffset));
			PVector br = Tracker.unMapPosition(PVector.add(gcenter, gridOffset));
			//PApplet.println("Drawing rect "+tl+" to "+br);
			laser.rect(tl.x,tl.y,(br.x-tl.x),(br.y-tl.y));
			laser.cellEnd(id);
		}
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
			ts=Ableton.getInstance().setTrackSet(theOscMessage.get(0).stringValue());
			handled=true;
		}
		if (!handled)
			super.handleMessage(theOscMessage);
	}
}

