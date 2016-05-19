import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class VisualizerProximity extends VisualizerPS {
	HashMap<Integer,Integer> assignments;
	String songs[]={"QU","DB","NG","FI","FO","GA","MB","EP","OL","PR","AN","MV"};
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
		Laser.getInstance().setFlag("body",1.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}
	
	public void stop() {
		super.stop();
		assignments.clear();
	}

	public int clipNumber(int nclips,int id1, int id2) {
		return (id1*7+id2)%nclips;
	}
	
	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
	//	HashMap<Integer,Integer> newAssignments=new HashMap<Integer,Integer>();
		for (Map.Entry<Integer,Person> e1: allpos.pmap.entrySet()) {
			int id1=e1.getKey();
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
			//PApplet.println("ID "+id1+" closest to id "+closest+" at a distance of "+Math.sqrt(mindist));

			if (current!=closest) {
				PApplet.println("ID "+closest+" now closer to id "+id1+" instead of "+current+" at a distance of "+Math.sqrt(mindist));

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
				PApplet.println("update: no update info for assignment for id "+id);
				i.remove();
			}
		}
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);

		g.textSize(0.16f);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		for (Map.Entry<Integer,Integer> entry: assignments. entrySet()) {
			int id1=entry.getKey();
			int id2=entry.getValue();
			if (id2!=-1) {
			//PApplet.println("grid "+cell+", id="+id+" "+gridColors.get(cell));
			g.fill(127,0,0,127);
			g.strokeWeight(0.05f);
			g.stroke(127,0,0);
			g.line((p.get(id1).getNormalizedPosition().x+1)*wsize.x/2, (p.get(id1).getNormalizedPosition().y+1)*wsize.y/2, (p.get(id2).getNormalizedPosition().x+1)*wsize.x/2, (p.get(id2).getNormalizedPosition().y+1)*wsize.y/2);
			}
		}
		g.fill(127);
		g.textAlign(PConstants.LEFT, PConstants.TOP);
		g.textSize(0.24f);
		g.text(ts.name,5,5);
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
//			PApplet.println("Drawing line "+p1+" to "+p2);
			laser.line(p1.x,p1.y,p2.x,p2.y);
			laser.shapeEnd("prox:"+id1+"-"+id2);
		}
		laser.bgEnd();
	}
	
	public void setnpeople(int n) {
		// Ignored for now
	}
}

