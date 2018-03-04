package com.pulsefield.tracker;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.logging.Logger;

import delaunay.Pnt;
import delaunay.Triangle;
import delaunay.Triangulation;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

class Voice {
	int id;
	PVector mainline[];  // Line for this note
	boolean playing;
    private final static Logger logger = Logger.getLogger(Voice.class.getName());

   
	Voice(int id) { 
		this.id=id; 
		mainline=new PVector[2];
		playing=false; 
	}
	void play(Scale scale, Synth synth, int duration, int channel) {
		if (mainline[0]==null || mainline[1]==null) {
			logger.warning("play: ID "+id+" has no vornoi line; mainline="+mainline[0]+","+mainline[1]);
			return;
		}
		PVector diff=new PVector(mainline[0].x,mainline[0].y);
		diff.sub(mainline[1]);
		float mag=diff.mag();
//		logger.fine("Mag="+mag); // Mag can be from 0 to 2*sqrt(2)
		mag=(mag>1.0)?1.0f:mag;
		int pitch=scale.map2note(1.0f-mag, 0f, 1.0f, 0, 3);
//		logger.fine("Pitch="+pitch);
		int velocity=127;
		synth.play(id, pitch, velocity, duration, channel);
	}
	
	boolean set(PVector p1, PVector p2) {
		final float maxval=0.9f;
		PVector mainline[]=new PVector[2];
		mainline[0]=new PVector(p1.x,p1.y);
		mainline[1]=new PVector(p2.x,p2.y);
		for (int i=0;i<2;i++) {
			if (mainline[i].x > maxval) {
				if (mainline[1-i].x>maxval)
					return false;
				else
					mainline[i]=PVector.lerp(mainline[1-i],mainline[i],(maxval-mainline[1-i].x)/(mainline[i].x-mainline[1-i].x));
			} else if (mainline[i].x < -maxval) {
				if (mainline[1-i].x < -maxval)
					return false;
				else
					mainline[i]=PVector.lerp(mainline[1-i],mainline[i],(mainline[1-i].x+maxval)/(mainline[1-i].x-mainline[i].x));
			}
			if (mainline[i].y > maxval) {
				if (mainline[1-i].y>maxval)
					return false;
				else
					mainline[i]=PVector.lerp(mainline[1-i],mainline[i],(maxval-mainline[1-i].y)/(mainline[i].y-mainline[1-i].y));
			} else if (mainline[i].y < -maxval) {
				if (mainline[1-i].y<-maxval)
					return false;
				else
					mainline[i]=PVector.lerp(mainline[1-i],mainline[i],(mainline[1-i].y+maxval)/(mainline[1-i].y-mainline[i].y));	
			}
		}
		//logger.fine("p1="+p1+"->"+mainline[0]);
		//logger.fine("p2="+p2+"->"+mainline[1]);
		this.mainline=mainline;
		return true;
	}
}

public class VisualizerVoronoi extends VisualizerPS {
	Triangle initialTriangle;
	final float initialSize=100f;  // Big enough to be outside active region
	Triangulation dt;
	List <Voice> voices;
	float last;
	final float noteDuration=0.5f;   // beats
	Voice curVoice;
	Synth synth;
	Scale scale;
	TrackSet trackSet;
	
	VisualizerVoronoi(PApplet parent, Scale scale, Synth synth) {
		super(parent);
		initialTriangle = new Triangle(
				new Pnt(-initialSize, -initialSize),
				new Pnt( initialSize, -initialSize),
				new Pnt(           0,  initialSize));
		voices=new ArrayList<Voice>();
		last=MasterClock.getBeat();
		this.synth=synth;
		curVoice=null;
		dt=new Triangulation(initialTriangle);
		this.scale=scale;
		logger.fine("VisualizerVoronoi() done");
	}

	@Override
	public void start() {
		logger.info("Voronoi::Start");
		super.start();
		Laser.getInstance().setFlag("body",1.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		trackSet=Ableton.getInstance().setTrackSet("Harp");
	}

	@Override
	public void stop() {
		logger.info("Voronoi::Stop");
		super.stop();
	}


	@SuppressWarnings("unused")
	@Override
	public void update(PApplet parent, People allpos) {
		super.update(parent, allpos);

		// Create Delaunay triangulation
		
		dt=new Triangulation(initialTriangle);
		// Put points in corners
		dt.delaunayPlace(new PntWithID(0,Tracker.minx,Tracker.miny));
		dt.delaunayPlace(new PntWithID(0,Tracker.minx,Tracker.maxy));
		dt.delaunayPlace(new PntWithID(0,Tracker.maxx,Tracker.miny));
		dt.delaunayPlace(new PntWithID(0,Tracker.maxx,Tracker.maxy));
		if (false) {
			float gapAngle=(float)(10f*Math.PI /180);
			float step=(float)(2*Math.PI-gapAngle)/8;
			for (float angle=gapAngle/2+step/2;angle<2*Math.PI;angle+=step) {
				Pnt ptsin=new Pnt((float)((Math.sin(angle+Math.PI))*0.99),(float)((Math.cos(angle+Math.PI))*0.99));
				Pnt ptsout=new Pnt((float)((Math.sin(angle+Math.PI))*1.01),(float)((Math.cos(angle+Math.PI))*1.01));
				dt.delaunayPlace(ptsin);
				dt.delaunayPlace(ptsout);
			}
		}

		for (Person p: allpos.pmap.values()) {
			dt.delaunayPlace(new PntWithID(p.id,p.getOriginInMeters().x,p.getOriginInMeters().y));
		}
		float beat=MasterClock.getBeat();
		if (beat-last >= noteDuration) {
			if (curVoice!=null)
				curVoice.playing=false;
			last=beat;
			Voice newvoice=null;
			Voice firstvoice=null;
			for (Voice v: voices) {
				if (firstvoice==null)
					firstvoice=v;
				if (curVoice==null || v.id > curVoice.id) {
					newvoice=v;
					break;
				}
			}
			if (newvoice==null)
				newvoice=firstvoice;
			if (newvoice!=null) {
				curVoice=newvoice;
				Person pos=allpos.get(curVoice.id);
				if (pos==null) {
					logger.fine("Delete voice for ID "+curVoice.id);
					voices.remove(curVoice);
					curVoice=null;
				} else {
					curVoice.play(scale, synth, (int)(noteDuration*480), pos.channel);
					curVoice.playing=true;
				}
			}
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People allpos) {
		final boolean showConnections=true;
		super.draw(t, g, allpos);
		if (allpos.pmap.isEmpty())
			return;
		
		// Draw Voronoi diagram
		// Keep track of sites done; no drawing for initial triangles sites
		HashSet<Pnt> done = new HashSet<Pnt>(initialTriangle);
		int tnum=0;
		for (Triangle triangle : dt) {
			if (showConnections){
				g.noFill();
				g.strokeWeight(0.04f);
				g.stroke(0,127,255);
				g.beginShape();
				for (int i=0;i<3;i++) {
					Pnt c=triangle.get(i);
					g.vertex((float)c.coord(0),(float)c.coord(1));
				}
				g.endShape(PConstants.CLOSE);
			}

			Pnt cc=triangle.getCircumcenter();
			g.fill(255);
			g.textAlign(PConstants.CENTER,PConstants.CENTER);
			//drawText(g,0.20f,"T"+tnum,(float)cc.coord(0),(float)cc.coord(1));
			tnum++;
			for (Pnt site: triangle) {
				if (site==null) {
					logger.severe("site=null");
					continue;
				}
				if (done.contains(site)) continue;
				done.add(site);
				PntWithID idsite=((PntWithID)site);


				List<Triangle> list = dt.surroundingTriangles(site, triangle);
				// Draw all the surrounding triangles
				g.noFill();
				g.stroke(0,255,0);
				g.strokeWeight(0.02f);
				g.beginShape();
				for (Triangle tri: list) {
					Pnt c=tri.getCircumcenter();
					g.vertex((float)c.coord(0),(float)c.coord(1));
				}
				g.endShape(PConstants.CLOSE);

				// Save one voronoi line as the note marker
				Voice v=null;
				for (int i=0;i<voices.size();i++)
					if (voices.get(i).id==idsite.id) {
						v=voices.get(i);
						break;
					}

				if (v == null) {
					v=new Voice(idsite.id);
					voices.add(v);
				}
				boolean hasLine=false;
				for (int i=0;i<list.size()-1;i++) {
					Pnt c1=list.get(i).getCircumcenter();
					Pnt c2=list.get(i+1).getCircumcenter();
					if (v.set(new PVector((float)c1.coord(0),(float)c1.coord(1)),new PVector((float)c2.coord(0),(float)c2.coord(1)))) {
						hasLine=true;
						break;
					}
				}

				// Draw the major line
				if (hasLine && v.playing) {
					g.stroke(allpos.get(idsite.id).getcolor());
					g.strokeWeight(0.05f);
					PVector scoord1=v.mainline[0];
					PVector scoord2=v.mainline[1];
					PVector path[]=vibratingPath(scoord1,scoord2,3,2.0f,0.3f,t.frameCount/t.frameRate);
					for (int i=0;i+3<path.length;i+=3)
						g.bezier(path[i].x,path[i].y,path[i+1].x,path[i+1].y,path[i+2].x,path[i+2].y,path[i+3].x,path[i+3].y);
				}
			}
		}
	}
	
	PVector pntToWorld(Pnt p) {
		return new PVector((float)p.coord(0),(float)p.coord(1));
	}
	
	// Draw to laser
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		Laser laser=Laser.getInstance();
		laser.bgBegin();
		

		// Draw Voronoi diagram
		// Keep track of sites done; no drawing for initial triangles sites
		HashSet<Pnt> done = new HashSet<Pnt>(initialTriangle);
		for (Triangle triangle : dt) {
//			for (int i=0;i<3;i++) {
//				PVector c1=pntToWorld(triangle.get(i));
//				PVector c2=pntToWorld(triangle.get((i+1)%3));
//				laser.line(c1.x, c1.y, c2.x, c2.y);
//			}


//			PVector cc=pntToWorld(triangle.getCircumcenter());
			laser.shapeBegin(triangle.toString());
			for (Pnt site: triangle) {
				if (done.contains(site)) continue;
				done.add(site);
				PntWithID idsite=((PntWithID)site);


				List<Triangle> list = dt.surroundingTriangles(site, triangle);
				// Draw all the surrounding triangles
				PVector prev=pntToWorld(list.get(list.size()-1).getCircumcenter());
				for (Triangle tri: list) {
					PVector c=pntToWorld(tri.getCircumcenter());
					laser.line(prev.x,prev.y,c.x,c.y);
					prev=c;
				}

				// Save one voronoi line as the note marker
				Voice v=null;
				for (int i=0;i<voices.size();i++)
					if (voices.get(i).id==idsite.id) {
						v=voices.get(i);
						break;
					}

				if (v == null) {
					v=new Voice(idsite.id);
					voices.add(v);
				}
				boolean hasLine=false;
				for (int i=0;i<list.size()-1;i++) {
					Pnt c1=list.get(i).getCircumcenter();
					Pnt c2=list.get(i+1).getCircumcenter();
					if (v.set(new PVector((float)c1.coord(0),(float)c1.coord(1)),new PVector((float)c2.coord(0),(float)c2.coord(1)))) {
						hasLine=true;
						break;
					}
				}

				// Draw the major line
				if (hasLine && v.playing) {
					PVector scoord1=v.mainline[0];
					PVector scoord2=v.mainline[1];
					PVector path[]=vibratingPath(scoord1,scoord2,3,1.0f,0.2f,parent.frameCount/parent.frameRate);
					for (int i=0;i+3<path.length;i+=3)
						laser.cubic(path[i].x,path[i].y,path[i+1].x,path[i+1].y,path[i+2].x,path[i+2].y,path[i+3].x,path[i+3].y);
				}
			}
			laser.shapeEnd(triangle.toString());
		}
		
		laser.bgEnd();
	}
}

