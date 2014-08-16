import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PShape;
import processing.core.PVector;

class Dancer {
	final float DAMPING=.05f;
	PVector neutral;
	PVector current;
	int score;
	final float MINMOVEDIST=.01f;
	
	Dancer(PVector pos) {
		neutral=new PVector();
		neutral.x=pos.x; neutral.y=pos.y;
		current=new PVector();
		current.x=pos.x; current.y=pos.y;
		score=0;
	}
	public void update(PVector newpos) {
		neutral.x=DAMPING*newpos.x+(1-DAMPING)*neutral.x;
		neutral.y=DAMPING*newpos.y+(1-DAMPING)*neutral.y;
		current.x=newpos.x;
		current.y=newpos.y;
	}
	public int getAim() {
		PVector offset=new PVector(current.x,current.y);
		offset.sub(neutral);
		float angle=offset.heading();
		float dist=offset.mag();
		int quad=(int)Math.round(angle/(Math.PI/2));
//		PApplet.println("Video: ID="+id+", current="+d.current+", quad="+quad+", dist="+dist);
		if (dist >= MINMOVEDIST)
			return (quad+4)%4;
		return -1;
	}
	public void incrementScore() {
		score++;
	}
	public void setScore(int s) { score=s; }
}

enum Direction {
	LEFT, RIGHT, UP, DOWN
}

class Song {
	String sfdir;
	String sfname;
	int clipNumber;
	int track;
	Simfile sf;
	
	Song(String sfdir, String sfname, int clipNumber)  {
		this.sfdir=sfdir;
		this.sfname=sfname;
		this.clipNumber=clipNumber;
		track=Ableton.getInstance().trackSet.firstTrack;
		PApplet.println("DDR: Adding song "+sfname+" at track "+track+", clip "+clipNumber);
		sf=null;
	}
	
	Simfile getSimfile() {
		if (sf==null) {
			sf=new Simfile();
			try {
				sf.loadSM(sfdir,sfname);
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
		}
		return sf;
	}
}

// Dance revolution visualizer
public class VisualizerDDR extends Visualizer {
	ArrayList<Song> songs;
	Simfile sf;
	long startTime;
	PImage arrow;   // Left pointing arrow
	PShape dancer;
	Song cursong=null;
	final float DOTSIZE=50f;
	float lastClipPosition;
	
	HashMap<Integer, Dancer> dancers;

	VisualizerDDR(PApplet parent) {
		super();
		dancers = new HashMap<Integer, Dancer>();
		arrow = parent.loadImage("arrow3.png");
		dancer=parent.loadShape(Tracker.SVGDIRECTORY+"dancer4.svg");
		assert(dancer!=null);
		songs=null;  // Initialize in start()
	}

	public void handleMessage(OscMessage msg) {
		PApplet.println("DDR message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		
		if (components.length<3 || !components[2].equals("ddr")) 
			PApplet.println("DDR: Expected /video/ddr messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("songnum")) {
			chooseSong((int)(msg.get(0).floatValue()*songs.size()));
		} else 
			PApplet.println("Unknown Navier Message: "+msg.toString());
	}
	
	/* Randomly choose a song */
	void chooseSong() {
		int songIndex = (int)(Math.random()*songs.size());
		chooseSong(songIndex);
	}
	
	void chooseSong(int songIndex) {
		if (songs.get(songIndex)==cursong)
			return;
		if (cursong!=null)
			Ableton.getInstance().stopClip(cursong.track,cursong.clipNumber);
		cursong=songs.get(songIndex);
		PApplet.println("Chose song "+songIndex+": "+cursong.getSimfile().toString());
		OscMessage msg=new OscMessage("/video/ddr/song");
		msg.add(cursong.sf.getTag("TITLE"));
		TouchOSC.getInstance().sendMessage(msg);
		Ableton.getInstance().playClip(cursong.track,cursong.clipNumber);
	}
	
	public void update(PApplet parent, People allpos) {
		if (allpos.pmap.isEmpty()) {
			if (cursong!=null) {
				Ableton.getInstance().stopClip(cursong.track,cursong.clipNumber);
				cursong=null;
			} 
		} else if (cursong==null)
			chooseSong();
			
		if (cursong==null)
			return;
		
		// Update internal state of the dancers
		for (int id: allpos.pmap.keySet()) {
			if (!dancers.containsKey(id))
				dancers.put(id,new Dancer(allpos.get(id).getNormalizedPosition()));
			PVector currentpos=allpos.get(id).getNormalizedPosition();
			dancers.get(id).update(currentpos);
			//PApplet.println("Dancer "+id+" moved to "+currentpos.toString());
		}
		// Remove dancers for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = dancers.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				iter.remove();
			}
		}
		beat();
	}

	
	// Called each time a beat passes by
	public void beat() {
		final int AIMS[]={2,3,1,0};
		// Note ordering in SIM file is left(0),up(1),down(2),right(3)
		// Ordering based on angle goes CCW starting from right, so it right(0),down(1),left(2),up(3)
		// Check current positions to see who is doing the right stuff
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		int pattern=cursong.getSimfile().findClosestDifficulty(0);
		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, lastClipPosition, clip.position);
		lastClipPosition=clip.position;
		for (NoteData n: notes) {
			PApplet.println("At clip time "+clip.position+", note timestamp "+n.timestamp+", notes="+n.notes);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					for (int id: dancers.keySet()) {
						Dancer d=dancers.get(id);
						PApplet.println("Dancer "+id+" has aim "+d.getAim());
						if (d.getAim() == AIMS[i])
							d.incrementScore();
						//d.setScore(d.getAim());
					}
				}
			}
		}
	}

	public void start() {
		super.start();
		startTime=System.currentTimeMillis();
		PApplet.println("Starting DDR at "+startTime);
		Ableton.getInstance().setTrackSet("DD");
		if (songs==null) {
			songs=new ArrayList<Song>();
			songs.add(new Song("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/StepMix 1.0/Impossible Fidelity/","impossible.sm",0));
			songs.add(new Song("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/Plaguemix Series/Super Trouper","supertrouper.sm",1));
			songs.add(new Song("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/Plaguemix Series/Krupa","krupa.sm",2));
		}
		cursong=null;
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}

	public void stop() {
		super.stop();
		PApplet.println("Stopping DDR at "+System.currentTimeMillis());
		if (cursong!=null) {
			Ableton.getInstance().stopClip(cursong.track,cursong.clipNumber);
			cursong=null;
		}
	}

	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent,p,wsize);
		if (p.pmap.isEmpty() || cursong==null)
			return;
		
		final float leftwidth=150;
		final float rightwidth=250;
		final float rightmargin=50;

		parent.imageMode(PConstants.CORNER);
		parent.image(cursong.getSimfile().getBanner(parent), wsize.x/4, 0, wsize.x/2, wsize.y/4);
		

		drawScores(parent,p,new PVector(leftwidth,wsize.y));
		parent.translate(leftwidth,0);
		drawPF(parent,p,new PVector(wsize.x-leftwidth-rightwidth,wsize.y));
		parent.translate(wsize.x-leftwidth-rightwidth,0);
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		if (clip!=null) {
//			PApplet.println("Clip at "+clip.position);
			drawTicker(parent,new PVector(rightwidth-rightmargin,wsize.y),clip.position);
		} else 
			PApplet.println("Ableton clip is null (track="+cursong.track+", clip="+cursong.clipNumber+")");
	}


	public void drawPF(PApplet parent, People allpos, PVector wsize) {
		final float ARROWSIZE=DOTSIZE;
		final float ARROWDIST=(ARROWSIZE+DOTSIZE)/2;

		drawBorders(parent,true,wsize);
		parent.imageMode(PConstants.CENTER);
		parent.tint(255);

		// Add drawing code here
		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			Person p=allpos.get(id);
			if (p==null) {
				PApplet.println("drawPF: Person "+id+" not found");
				continue;
			}
			int quad=d.getAim();
			parent.pushMatrix();
			parent.translate((d.neutral.x+1)*wsize.x/2,(d.neutral.y+1)*wsize.y/2);
			parent.fill(p.getcolor(parent));
			parent.ellipse(0,0,DOTSIZE,DOTSIZE);
//			PApplet.println("Video: ID="+id+", current="+d.current+", quad="+quad+", dist="+dist);
			if (quad>=0) {
				parent.rotate((float)(quad*Math.PI/2+Math.PI));
				parent.translate(-ARROWDIST, 0);
				parent.image(arrow, 0, 0, ARROWSIZE, ARROWSIZE);
			} else {
				//parent.shape(dancer, 0, 0, ARROWSIZE, ARROWSIZE);
			}
			parent.popMatrix();
		}
	}

	public void drawScores(PApplet parent, People allpos, PVector wsize) {
		float lineHeight=wsize.y/12;

		parent.stroke(255);
		parent.fill(255);
		parent.tint(255);
		parent.pushMatrix();
		
		
		parent.textSize(24);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.translate(0,lineHeight/2);
		parent.text("SCORES",wsize.x/2,0);
		parent.translate(0,lineHeight);
		parent.textAlign(PConstants.LEFT,PConstants.CENTER);
		parent.textSize(16);

		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			Person p=allpos.get(id);
			if (p==null)
				continue;
			
			parent.fill(p.getcolor(parent));
			parent.ellipse(DOTSIZE/2,0,DOTSIZE/2, DOTSIZE/2);
			parent.fill(255);
			parent.text(""+d.score,DOTSIZE*1.5f,0);
			parent.translate(0, lineHeight);
		}
		parent.popMatrix();

	}

	public void drawTicker(PApplet parent, PVector wsize, float now) {
		final float DURATION=6.0f;  // Duration of display top to bottom
		final float HISTORY=2f;    // Amount of past showing
		int pattern=cursong.getSimfile().findClosestDifficulty(0);
		float songdur=cursong.getSimfile().getduration(pattern);
		if (now>songdur) {
			PApplet.println("Song duration "+songdur+" ended");
			((Tracker)parent).cycle();
		}
	
		if (cursong==null) {
			PApplet.println("cursong=null");
			return;
		}
		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, now-HISTORY, now-HISTORY+DURATION);
//		PApplet.println("Have "+notes.size()+" notes.");
		//parent.ellipse(wsize.x/2,wsize.y/2,100,100);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.textSize(16);
		parent.tint(255);
		parent.stroke(255);
		parent.fill(255);

		float arrowsize=wsize.x/4f/1.2f;
		for (NoteData n: notes) {
			final float angles[]={0f,-(float)(Math.PI/2),-(float)(3*Math.PI/2),-(float)Math.PI};
			float ypos=(n.timestamp-(now-HISTORY))/DURATION*wsize.y;
			//PApplet.println("At "+n.timestamp+", notes="+n.notes+", y="+ypos);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					float xpos=(i+0.5f)*wsize.x/n.notes.length();
					//PApplet.println("x="+xpos+", text="+n.notes.substring(i,i+1));
					//parent.text(n.notes.substring(i,i+1),xpos,ypos);
					parent.pushMatrix();
					parent.translate(xpos,ypos);
					parent.rotate(angles[i]);
					parent.image(arrow, 0, 0, arrowsize, arrowsize);
					parent.popMatrix();
				}
			}
		}
		parent.strokeWeight(5);
		parent.line(0,HISTORY*wsize.y/DURATION,0,wsize.x,HISTORY*wsize.y/DURATION,0);
		parent.text(String.format("%.2f", now), 5, wsize.y-10);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent, p);
		
		if (p.pmap.isEmpty())
			return;

		drawLaserPF(parent,p);
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		if (clip!=null) {
//			PApplet.println("Clip at "+clip.position);
			drawLaserTicker(parent,clip.position);
		} else 
			PApplet.println("Ableton clip is null (track="+cursong.track+", clip="+cursong.clipNumber+")");
	}

	public void drawLaserPF(PApplet parent, People allpos) {
		Laser laser=Laser.getInstance();
		
		for (int id: dancers.keySet()) {
			laser.cellBegin(id);
			Dancer d=dancers.get(id);

			int quad=d.getAim();
			//PApplet.println("Laser: ID="+id+", current="+d.current+", quad="+quad+", dist="+dist);
			if (quad>=0)
				laser.svgfile("arrow4.svg", 0.0f, 0.0f, 0.5f, quad*90+180);
			else
				laser.svgfile("dancer4.svg", 0.0f, 0.0f, 0.5f,0f);				
			laser.cellEnd(id);
		}
	}


	public void drawLaserTicker(PApplet parent, float now) {
		final float TICKERWIDTH=1.5f;	// Width of ticker in meters (centered)
		final float ARROWSIZE=TICKERWIDTH/6;
		final float DURATION=5.0f;  // Duration of display top to bottom (seconds)
		final float HISTORY=1.0f;    // Amount of past showing
		int pattern=cursong.getSimfile().findClosestDifficulty(0);


		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, now-HISTORY, now-HISTORY+DURATION);

		Laser laser=Laser.getInstance();
		laser.bgBegin();
		// Draw the zero-line arrows
		final float angles[]={0f,(float)(Math.PI/2),(float)(3*Math.PI/2),(float)Math.PI};

		float yposzero=Tracker.rawminy+HISTORY/DURATION*(Tracker.rawmaxy-Tracker.rawminy);
		for (int i=0;i<angles.length;i++) {
			float xpos=((((float)i)/(angles.length-1))-0.5f)*TICKERWIDTH;
			laser.svgfile("arrow4.svg",xpos,yposzero,ARROWSIZE*1.5f,(float)(angles[i]*180/Math.PI));
		}
		for (NoteData n: notes) {
			float ypos=Tracker.rawminy+(n.timestamp-(now-HISTORY))/DURATION*(Tracker.rawmaxy-Tracker.rawminy);
			//PApplet.println("At "+n.timestamp+", notes="+n.notes+", y="+ypos);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					float xpos=(Tracker.rawminx+Tracker.rawmaxx)/2.0f - ((((float)i)/(n.notes.length()-1))-0.5f)*TICKERWIDTH;
					laser.svgfile("arrow4.svg",xpos,ypos,ARROWSIZE,(float)(180+angles[i]*180/Math.PI));
				}
			}
		}
		laser.bgEnd();
	}
}

