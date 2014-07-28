import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

public class VisualizerProximity extends VisualizerPS {
	HashMap<Integer,Integer> assignments;
	String songs[]={"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR"};
	int song=0;
	TrackSet ts;
	static final float MAXSEP=0.2f; // Maximum separation to trigger
	
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
		PApplet.println("Starting proximity with song "+song+": "+ts.name);
	}
	
	public void stop() {
		super.stop();
		assignments.clear();
	}

	public int clipNumber(int nclips,int id1, int id2) {
		return (id1*7+id2)%nclips;
	}
	
	public void update(PApplet parent, Positions allpos) {
		super.update(parent,allpos);
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Map.Entry<Integer,Position> e1: allpos.positions.entrySet()) {
			int id1=e1.getKey();
			PVector pos1=e1.getValue().origin;
			// Find closest NEIGHBOR	
			int closest=-1;
			double mindist=MAXSEP;
			int current=-1;
			if (assignments.containsKey(id1))
				current=assignments.get(id1);
			
			for (Map.Entry<Integer,Position> e2: allpos.positions.entrySet()) {
				int id2=e2.getKey();
				if (id1==id2)
					continue;
				PVector pos2=e2.getValue().origin;
				
				double dist2=Math.pow(pos1.x-pos2.x,2)+Math.pow(pos1.y-pos2.y,2);
				if (id2==current)
					dist2*=0.9;  // Hysteresis
				
				if (dist2 < mindist) {
					mindist=dist2;
					closest=id2;
				}
			}
			//PApplet.println("ID "+id1+" closest to id "+closest+" at a distance of "+Math.sqrt(mindist));

			if (current!=closest) {
				PApplet.println("ID "+closest+" now closer to id "+id1+" instead of "+current+" at a distance of "+Math.sqrt(mindist));

				int track=id1%(ts.numTracks)+ts.firstTrack;
//				if (current!=-1)
//					Ableton.getInstance().stopClip(track, clipNumber(id1,closest));
				assignments.put(id1,closest);
				if (closest!=-1) {
					int nclips=Ableton.getInstance().getTrack(track).numClips();
					Ableton.getInstance().playClip(track,clipNumber(nclips,id1,closest));
				}
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
			int id1=entry.getKey();
			int id2=entry.getValue();
			if (id2!=-1) {
			//PApplet.println("grid "+cell+", id="+id+" "+gridColors.get(cell));
			parent.fill(127,0,0,127);
			parent.strokeWeight(5);
			parent.stroke(127,0,0);
			parent.line((p.get(id1).origin.x+1)*wsize.x/2, (p.get(id1).origin.y+1)*wsize.y/2, (p.get(id2).origin.x+1)*wsize.x/2, (p.get(id2).origin.y+1)*wsize.y/2);
			}
		}
		parent.fill(127);
		parent.textAlign(PConstants.LEFT, PConstants.TOP);
		parent.textSize(24);
		parent.text(ts.name,5,5);
	}

	public void drawLaser(PApplet parent, Positions p) {
		super.drawLaser(parent,p);
		Laser laser=Laser.getInstance();
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id1=entry.getKey();
			int id2=entry.getValue();
			if (id2==-1)
				continue;
			laser.cellBegin(id1);
			PVector p1 = Tracker.unMapPosition(p.get(id1).origin);
			PVector p2 = Tracker.unMapPosition(p.get(id2).origin);
//			PApplet.println("Drawing line "+p1+" to "+p2);
			laser.line(p1.x,p1.y,p2.x,p2.y);
			laser.cellEnd(id1);
		}
	}
	
	public void setnpeople(int n) {
		// Ignored for now
	}
}

