import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;

class Dancer {
	final float DAMPING=.05f;
	PVector neutral;
	PVector current;
	int score;

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
		track=91;
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
	final float MINMOVEDIST=.01f;
	ArrayList<Song> songs;
	Simfile sf;
	long startTime;
	PImage arrow;
	Song cursong=null;
	final float DOTSIZE=50f;
	boolean active=false;
	
	HashMap<Integer, Dancer> dancers;

	VisualizerDDR(PApplet parent) {
		super();
		dancers = new HashMap<Integer, Dancer>();
		arrow = parent.loadImage("arrow3.png");
		songs=new ArrayList<Song>();
		songs.add(new Song("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/StepMix 1.0/Impossible Fidelity/","impossible.sm",0));
		songs.add(new Song("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/Plaguemix Series/Super Trouper","supertrouper.sm",1));
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
		if (active)
			Ableton.getInstance().playClip(cursong.track,cursong.clipNumber);
	}
	
	public void update(PApplet parent, People allpos) {
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
	}

	public void beat() {
		// Called each time a beat passes by
	}

	public void start() {
		super.start();
		startTime=System.currentTimeMillis();
		PApplet.println("Starting DDR at "+startTime);
		active=true;
		chooseSong();
	}

	public void stop() {
		super.stop();
		PApplet.println("Stopping DDR at "+System.currentTimeMillis());
		Ableton.getInstance().stopClip(cursong.track,cursong.clipNumber);
		active=false;
		cursong=null;
	}

	public void draw(PApplet parent, People p, PVector wsize) {
		final float leftwidth=150;
		final float rightwidth=250;
		final float rightmargin=50;

		parent.background(0, 0, 0);  
		parent.imageMode(PConstants.CORNERS);
		//parent.image(banner, 0, 0, wsize.x, wsize.y);
		parent.colorMode(PConstants.RGB, 255);
		if (p.pmap.isEmpty())
			drawWelcome(parent,wsize);

		drawScores(parent,p,new PVector(leftwidth,wsize.y));
		parent.translate(leftwidth,0);
		drawPF(parent,p,new PVector(wsize.x-leftwidth-rightwidth,wsize.y));
		parent.translate(wsize.x-leftwidth-rightwidth,0);
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		if (clip!=null) {
			//PApplet.println("Clip at "+clip.position);
			drawTicker(parent,new PVector(rightwidth-rightmargin,wsize.y),clip.position/Ableton.getInstance().getTempo()*60f);
		}
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
			PVector offset = d.current;
			offset.sub(d.neutral);
			float angle=offset.heading();
			float dist=offset.mag();
			int quad=(int)Math.round(angle/(Math.PI/2));
			parent.pushMatrix();
			parent.translate((d.neutral.x+1)*wsize.x/2,(d.neutral.y+1)*wsize.y/2);
			parent.fill(p.getcolor(parent));
			parent.ellipse(0,0,DOTSIZE,DOTSIZE);
			parent.rotate((float)(quad*Math.PI/2+Math.PI));
			parent.translate(-ARROWDIST, 0);
			if (dist >= MINMOVEDIST)
				parent.image(arrow, 0, 0, ARROWSIZE, ARROWSIZE);
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
	

		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, now-HISTORY, now-HISTORY+DURATION);
		
		//parent.ellipse(wsize.x/2,wsize.y/2,100,100);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		parent.textSize(16);
		parent.tint(255);
		parent.stroke(255);
		parent.fill(255);

		float arrowsize=wsize.x/4f/1.2f;
		for (NoteData n: notes) {
			final float angles[]={0f,(float)(Math.PI/2),(float)(3*Math.PI/2),(float)Math.PI};
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
}

