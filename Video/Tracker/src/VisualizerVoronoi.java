import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import delaunay.Pnt;
import delaunay.Triangle;
import delaunay.Triangulation;

class Voice {
	int id;
	PVector mainline[];  // Line for this note
	boolean playing;
	Voice(int id) { 
		this.id=id; 
		mainline=new PVector[2];
		playing=false; 
	}
	void play(Scale scale, Synth synth, int duration, int channel) {
		if (mainline[0]==null || mainline[1]==null) {
			PApplet.println("Voice.Play: ID "+id+" has no vornoi line");
			return;
		}
		PVector diff=new PVector(mainline[0].x,mainline[0].y);
		diff.sub(mainline[1]);
		float mag=diff.mag();
		//PApplet.println("Mag="+mag); // Mag can be from 0 to 2*sqrt(2)
		mag=(mag>1.5)?1.5f:mag;
		int pitch=scale.map2note(1.5f-mag, 0f, 1.5f, 0, 3);
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
		//PApplet.println("p1="+p1+"->"+mainline[0]);
		//PApplet.println("p2="+p2+"->"+mainline[1]);
		this.mainline=mainline;
		return true;
	}
}

public class VisualizerVoronoi extends VisualizerPS {
	Triangle initialTriangle;
	final int initialSize=3;
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
	}

	@Override
	public void start() {
		super.start();
		trackSet=Ableton.getInstance().setTrackSet("Harp");
	}

	@Override
	public void stop() {
		super.stop();
	}


	@SuppressWarnings("unused")
	@Override
	public void update(PApplet parent, People allpos) {
		super.update(parent, allpos);

		// Create Delaunay triangulation
		dt=new Triangulation(initialTriangle);

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
			dt.delaunayPlace(new PntWithID(p.id,p.getNormalizedPosition().x,p.getNormalizedPosition().y));
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
					PApplet.println("Delete voice for ID "+curVoice.id);
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
	public void draw(PApplet parent, People allpos, PVector wsize) {
		super.draw(parent, allpos, wsize);

		// Draw Voronoi diagram
		// Keep track of sites done; no drawing for initial triangles sites
		HashSet<Pnt> done = new HashSet<Pnt>(initialTriangle);
		int tnum=0;
		for (Triangle triangle : dt) {
			parent.noFill();
			parent.stroke(0,0,255);
			parent.strokeWeight(1);
			parent.beginShape();
			for (int i=0;i<3;i++) {
				Pnt c=triangle.get(i);
				parent.vertex((float)((c.coord(0)+1)*wsize.x/2),(float)((c.coord(1)+1)*wsize.y/2));
			}
			parent.endShape(PConstants.CLOSE);

			Pnt cc=triangle.getCircumcenter();
			parent.fill(255);
			parent.textAlign(PConstants.CENTER,PConstants.CENTER);
			parent.textSize(20);
			parent.text("T"+tnum,(float)(cc.coord(0)+1)*wsize.x/2f,(float)(cc.coord(1)+1)*wsize.y/2f);
			tnum++;
			for (Pnt site: triangle) {
				if (done.contains(site)) continue;
				done.add(site);
				PntWithID idsite=((PntWithID)site);


				List<Triangle> list = dt.surroundingTriangles(site, triangle);
				// Draw all the surrounding triangles
				parent.noFill();
				parent.stroke(0,255,0);
				parent.strokeWeight(1);
				parent.beginShape();
				for (Triangle tri: list) {
					Pnt c=tri.getCircumcenter();
					parent.vertex((float)((c.coord(0)+1)*wsize.x/2),(float)((c.coord(1)+1)*wsize.y/2));
				}
				parent.endShape(PConstants.CLOSE);

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
					parent.stroke(allpos.get(idsite.id).getcolor(parent));
					parent.strokeWeight(5);
					float rx=parent.randomGaussian()*3;
					float ry=parent.randomGaussian()*3;
					PVector scoord1=convertToScreen(v.mainline[0],wsize);
					PVector scoord2=convertToScreen(v.mainline[1],wsize);
					parent.line(scoord1.x+rx, scoord1.y+ry, scoord2.x+rx, scoord2.y+ry);
				}
			}
		}
	}
}

