package com.pulsefield.tracker;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Logger;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PShape;
import processing.core.PVector;

class Dancer {
	final float DAMPING=.05f;
	PVector neutral;
	PVector current;
	int score;
	final float MINMOVEDIST=.01f;
	int hit;   // Non-zero while they match the expected move
	int hitIconNumber; 
	
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
//		logger.fine("Video: ID="+id+", current="+d.current+", quad="+quad+", dist="+dist);
		if (dist >= MINMOVEDIST)
			return (quad+4)%4;
		return -1;
	}
	public void incrementScore() {
		score++;
	}
	public void setScore(int s) { score=s; }
	public void setHit(boolean isHit) { if (isHit) hit=1; else hit=Math.max(0,hit-1); }
	public boolean isHit() { return hit>0; }
	public void setHitIconNumber(int i) { hitIconNumber=i; }
	public int getHitIconNumber() { return hitIconNumber; }
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
    private final static Logger logger = Logger.getLogger(Song.class.getName());

	Song(String sfdir, String sfname, int clipNumber)  {
		this.sfdir=sfdir;
		this.sfname=sfname;
		this.clipNumber=clipNumber;
		track=Ableton.getInstance().trackSet.firstTrack;
		logger.fine("DDR: Adding song "+sfname+" at track "+track+", clip "+clipNumber);
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
	PImage arrowHit;   // Same, but for when the player hits the correct move
	PShape hitIcons[];
	PShape dancer;
	Images dancerImages;
	Song cursong=null;
	int pattern=0;  // Current pattern
	float lastClipPosition;
	final int targetDifficulty=4;
	int delayCounter=0;  // Counter to delay start of next song after one ends
	
	HashMap<Integer, Dancer> dancers;

	VisualizerDDR(PApplet parent) {
		super();
		dancers = new HashMap<Integer, Dancer>();
		arrow = parent.loadImage("DDR/arrow.png");
		arrowHit = parent.loadImage("DDR/arrowHit.png");
		dancerImages = new Images("DDR/dancers");
		dancer=parent.loadShape(Tracker.SVGDIRECTORY+"dancer4.svg");
		hitIcons = new PShape[4];
		for (int i=0;i<hitIcons.length;i++)
			hitIcons[i]=parent.loadShape(Tracker.SVGDIRECTORY+"ddrhit"+(i+1)+".svg");
		assert(dancer!=null);
		songs=null;  // Initialize in start()
	}

	public void handleMessage(OscMessage msg) {
		logger.fine("DDR message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		
		if (components.length<3 || !components[2].equals("ddr")) 
			logger.warning("DDR: Expected /video/ddr messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("songnum")) {
			if (songs!=null)
				chooseSong((int)(msg.get(0).floatValue()*songs.size()));
		} else 
			logger.warning("Unknown DDR Message: "+msg.toString());
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
		pattern=cursong.getSimfile().findClosestDifficulty(targetDifficulty);
		logger.info("Chose song "+songIndex+" with pattern "+pattern+", difficulty="+cursong.getSimfile().notes.get(pattern).difficultyMeter);
		logger.fine(cursong.getSimfile().toString());
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
				dancers.put(id,new Dancer(allpos.get(id).getOriginInMeters()));
			PVector currentpos=allpos.get(id).getOriginInMeters();
			dancers.get(id).update(currentpos);
			//logger.fine("Dancer "+id+" moved to "+currentpos.toString());
		}
		// Remove dancers for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = dancers.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				logger.fine("Removing ID "+id);
				iter.remove();
			}
		}
		beat();
	}

	
	// Called each time a beat passes by
	public void beat() {
		final int AIMS[]={2,1,3,0};
		// Note ordering in SIM file is left(0),up(1),down(2),right(3)
		// Ordering based on angle goes CCW starting from right, so it right(0),up(1),left(2),down(3)
		// Check current positions to see who is doing the right stuff
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, lastClipPosition, clip.position);
		lastClipPosition=clip.position;
		for (NoteData n: notes) {
			logger.fine("At clip time "+clip.position+", note timestamp "+n.timestamp+", notes="+n.notes);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					for (int id: dancers.keySet()) {
						Dancer d=dancers.get(id);
//						logger.fine("Dancer "+id+" has aim "+d.getAim());
						if (d.getAim() == AIMS[i]) {
							d.setHit(true);
							d.setHitIconNumber((int)(Math.random()*hitIcons.length));
							d.incrementScore();
						} else {
							d.setHit(false);
						}
						//d.setScore(d.getAim());
					}
				}
			}
		}
	}

	public void start() {
		super.start();
		startTime=System.currentTimeMillis();
		logger.info("Starting DDR at "+startTime);
		Ableton.getInstance().setTrackSet("DD");
		if (songs==null) {
			songs=new ArrayList<Song>();
			final String rootDir=Tracker.pfroot+"/../StepMania/Songs/";
			int clipCntr=0;
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/AXEL F","AXEL F.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Beat It","Beat It.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/CRAZY IN LOVE - Beyonce feat Jay Z","CRAZY IN LOVE - Beyonce feat Jay Z.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/DO SOMETHING - BRITNEY SPEARS","DO SOMETHING - BRITNEY SPEARS.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Fergalicious","Fergalicious.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Girls Just Wanna Have Fun","Girls Just Wanna Have Fun.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/I Gotta Feeling","I Gotta Feeling.sm",clipCntr++));
			songs.add(new Song(rootDir+"StepMix 1.0/Impossible Fidelity/","impossible.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Just Dance","Just Dance.sm",clipCntr++));
			songs.add(new Song(rootDir+"Plaguemix Series/Krupa","krupa.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/KUNG FU FIGHTING","KUNG FU FIGHTING.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/La Bamba","La Bamba.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/MORNING TRAIN - Sheena Easton","MORNING TRAIN - Sheena Easton.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/My heart will go on","heart.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/NEVER ENDING STORY","NEVER ENDING STORY.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/ONLY YOU","ONLY YOU.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/S.O.S","S.O.S..sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Shes a Maniac-ITGREADY","steps.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/SHUT IT DOWN","SHUT IT DOWN.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/SORRY - MADONNA","SORRY - MADONNA.sm",clipCntr++));
			songs.add(new Song(rootDir+"Plaguemix Series/Super Trouper","supertrouper.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Telephone","Lady Gaga feat. Beyoce - Telephone.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/THAT'S THE WAY (I LIKE IT)","THAT'S THE WAY (I LIKE IT).sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Toxic","Toxic.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/TWILIGHT ZONE (R-C Extended Club MIX)","TWILIGHT ZONE (R-C Extended Club MIX).sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Video Killed The Radio Star","Video Killed The Radio Star.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/WE WILL ROCK YOU","WE WILL ROCK YOU.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/WHERE IS THE LOVE","where.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Wherever You Are","Wherever You Are.sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/Y.M.C.A","Y.M.C.A..sm",clipCntr++));
			songs.add(new Song(rootDir+"stepmaniasong.xf.cz/You Sexy Thing","You Sexy Thing.sm",clipCntr++));			
			assert(clipCntr==31); // To check against Ableton
		}
		cursong=null;
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}

	public void stop() {
		super.stop();
		logger.info("Stopping DDR at "+System.currentTimeMillis());
		if (cursong!=null) {
			Ableton.getInstance().stopClip(cursong.track,cursong.clipNumber);
			cursong=null;
		}
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g,p);
		if (p.pmap.isEmpty() || cursong==null)
			return;

		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		if (clip==null) {
			logger.warning("DDR.draw: Ableton clip is null (track="+cursong.track+", clip="+cursong.clipNumber+")");
			return;
		}
		PVector center=Tracker.getFloorCenter();
		PImage banner=cursong.getSimfile().getBanner(t,g);
		
		if (clip.position < 10f && banner!=null) { // Show banner for first several seconds
			final float bannerWidth = 3f;   // Banner width in meters
			g.imageMode(PConstants.CENTER);
			float bannerHeight = banner.height*bannerWidth/banner.width;
			g.pushMatrix();
			g.translate(center.x, bannerHeight/2+Tracker.miny);
			g.scale(-1,1);  // Flip to RH coord system
			g.image(banner, 0, 0, bannerWidth, bannerHeight);
			g.popMatrix();	
		}
		

		//drawScores(g,p);
		drawPF(g,p);
//			logger.fine("Clip at "+clip.position);
			drawTicker(g,clip.position);
		float songdur=cursong.getSimfile().getduration(pattern);
		if ((clip.state==1 && clip.position>1) || clip.position>songdur) {
			delayCounter+=1;
			if (delayCounter>150) {
				logger.info("Song duration "+songdur+" ended; clip Position="+clip.position+", songdur="+songdur+", state="+clip.state);
				chooseSong();
				for (int id: dancers.keySet()) {
					Dancer d=dancers.get(id);
					d.score=0;
				}
			}
		} else
			delayCounter=0;
	}


	public void drawPF(PGraphics g, People allpos) {
		final float DOTSIZE=0.5f;  // Dotsize in meters
		final float ARROWSIZE=DOTSIZE;
		final float ARROWDIST=(ARROWSIZE+DOTSIZE)/2;
		final float scoreHeight=DOTSIZE*0.7f;

		//drawBorders(g);
		g.imageMode(PConstants.CENTER);
		g.tint(255);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		// Find current winner
		int bestScore=-1;
		int bestId=-1;
		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			if (d.score>bestScore) {
				bestScore=d.score;
				bestId=id;
			}
		}
		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			Person p=allpos.get(id);
			if (p==null) {
				logger.warning("drawPF: Person "+id+" not found");
				continue;
			}
			int quad=d.getAim();
			g.pushMatrix();
			g.translate(d.neutral.x,d.neutral.y);
//			g.fill(p.getcolor());
//			g.stroke(p.getcolor());
//			g.ellipse(0,0,DOTSIZE,DOTSIZE);
			PImage img=dancerImages.get(id);
			g.image(dancerImages.get(id),0,0,DOTSIZE,DOTSIZE*img.height/img.width);

			// Draw score below
			g.pushMatrix();
			g.translate(-DOTSIZE, -DOTSIZE);
			g.stroke(0);
			if (id==bestId)
				g.fill(0,255,0);
			else
				g.fill(255,0,0);
			float sh=scoreHeight;
			if (id==bestId && delayCounter>0)
				sh *= 3;  // Winner's score is in green and bigger
			drawText(g,sh,""+d.score,0,-sh/10);
			g.popMatrix();
			
			//logger.fine("Video: ID="+id+", current="+d.current+", quad="+quad);
			if (quad>=0) {
				g.rotate((float)(quad*Math.PI/2+Math.PI));
				g.translate(-ARROWDIST, 0);
				g.image(d.isHit()?arrowHit:arrow, 0, 0, ARROWSIZE, ARROWSIZE);
			} else {
				//parent.shape(dancer, 0, 0, ARROWSIZE, ARROWSIZE);
			}

			g.popMatrix();
		}
	}

	public void drawScores(PGraphics g, People allpos) {
		final float DOTSIZE=50f/Tracker.getPixelsPerMeter();
		final float textHeight=0.16f;
		final float lineHeight=textHeight*1.2f;
		PVector center=Tracker.getFloorCenter();
		PVector sz=Tracker.getFloorSize();
		final float firstLine=center.y-sz.y/2+textHeight;
		final float leftMargin=center.x-sz.x/2+0.2f;
		g.stroke(255);
		g.fill(255);
		g.tint(255);
		g.pushMatrix();
		
		
		g.textAlign(PConstants.LEFT,PConstants.CENTER);
		g.translate(-sz.x/2+leftMargin,-sz.y/2+firstLine);
		drawText(g,textHeight*1.5f,"SCORES",0,0);
		g.translate(0,lineHeight*1.5f);
		g.textAlign(PConstants.LEFT,PConstants.CENTER);
		g.ellipseMode(PConstants.CENTER);
		g.imageMode(PConstants.CENTER);
		g.textMode(PConstants.CENTER);
		for (int id: dancers.keySet()) {
			Dancer d=dancers.get(id);
			Person p=allpos.get(id);
			if (p==null)
				continue;
			
			g.fill(p.getcolor());
			g.ellipse(DOTSIZE/2,0,DOTSIZE/2, DOTSIZE/2);
			g.fill(255);
			drawText(g,0.16f,""+d.score,DOTSIZE*1.5f,0);
			g.translate(0, lineHeight);
		}
		g.popMatrix();
	}

	public void drawTicker(PGraphics g, float now) {
		final float DURATION=8.0f;  // Duration of display top to bottom
		final float HISTORY=1.0f;    // Amount of past showing
		final float tickerWidth=1.5f;  // Width of ticker in meters
		final float tickerTopMargin=1f;   // Ticker ends this distance from top of active area
		PVector sz=Tracker.getFloorSize();
		PVector center=Tracker.getFloorCenter();
		
		if (cursong==null) {
			logger.fine("cursong=null");
			return;
		}
		float songdur=cursong.getSimfile().getduration(pattern);
		if (now>songdur) {
			logger.fine("Song duration "+songdur+" ended");
//			((Tracker)parent).setapp(4);
		}
	

		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, now-HISTORY, now-HISTORY+DURATION);
//		logger.fine("Have "+notes.size()+" notes.");
		//parent.ellipse(wsize.x/2,wsize.y/2,100,100);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.tint(255);
		g.stroke(255);
		g.fill(255);

		float arrowsize=tickerWidth/5;
		for (NoteData n: notes) {
			final float angles[]={0f,-(float)(Math.PI/2),-(float)(3*Math.PI/2),-(float)Math.PI};
			float rel=(n.timestamp-(now-HISTORY))/DURATION;
			float ypos=(rel-0.5f)*(sz.y-tickerTopMargin)+(center.y+tickerTopMargin/2);
			//logger.fine("At "+n.timestamp+", rel="+rel+", notes="+n.notes+", y="+ypos+",sz.y="+sz.y+",center.y="+center.y);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					float xpos=center.x-tickerWidth/2+(i+0.5f)*tickerWidth/n.notes.length();
					//logger.fine("x="+xpos+", text="+n.notes.substring(i,i+1));
					//parent.text(n.notes.substring(i,i+1),xpos,ypos);
					g.pushMatrix();
					g.translate(xpos,ypos);
					g.rotate(angles[i]);
					g.image(arrow, 0, 0, arrowsize, arrowsize);
					g.popMatrix();
				}
			}
		}
		g.strokeWeight(0.05f);
		float ypos=(HISTORY/DURATION-0.5f)*(sz.y-tickerTopMargin)+(center.y+tickerTopMargin/2);
		g.line(center.x-tickerWidth/2,ypos,0,center.x+tickerWidth/2,ypos,0);
		drawText(g,0.16f,String.format("%.2f", now), center.x,ypos-0.2f);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent, p);
		
		if (p.pmap.isEmpty())
			return;

		drawLaserPF(parent,p);
		if (cursong==null)
			return;
		Clip clip=Ableton.getInstance().getClip(cursong.track, cursong.clipNumber);
		if (clip!=null) {
//			logger.fine("Clip at "+clip.position);
			drawLaserTicker(parent,clip.position);
		} else 
			logger.warning("Ableton clip is null (track="+cursong.track+", clip="+cursong.clipNumber+")");
	}

	public void drawLaserPF(PApplet parent, People allpos) {
		Laser laser=Laser.getInstance();
		
		for (int id: dancers.keySet()) {
			laser.cellBegin(id);
			Dancer d=dancers.get(id);

			int quad=d.getAim();
			//logger.fine("Laser: ID="+id+", current="+d.current+", quad="+quad+", dist="+dist);
			if (d.isHit())
				laser.svgfile("ddrhit"+(d.getHitIconNumber()+1)+".svg",0,-0.5f,0.5f,0f);
			else if (quad>=0)
				laser.svgfile("arrow4.svg", 0.0f, -0.5f, 0.5f, -quad*90+180);
			else
				laser.svgfile("dancer4.svg", 0.0f, -0.5f, 0.5f,0f);				
			laser.cellEnd(id);
		}
	}


	public void drawLaserTicker(PApplet parent, float now) {
		final float TICKERWIDTH=1.5f;	// Width of ticker in meters (centered)
		final float ARROWSIZE=TICKERWIDTH/6;
		final float DURATION=5.0f;  // Duration of display top to bottom (seconds)
		final float HISTORY=1.0f;    // Amount of past showing

		ArrayList<NoteData> notes=cursong.getSimfile().getNotes(pattern, now-HISTORY, now-HISTORY+DURATION);

		Laser laser=Laser.getInstance();
		laser.bgBegin();
		// Draw the zero-line arrows
		final float angles[]={(float)0f,(float)(Math.PI/2),(float)(Math.PI*3/2),(float)Math.PI};

		float yposzero=Tracker.miny+HISTORY/DURATION*(Tracker.maxy-Tracker.miny);
		for (int i=0;i<angles.length;i++) {
			float xpos=(Tracker.minx+Tracker.maxx)/2.0f - ((((float)i)/(angles.length-1))-0.5f)*TICKERWIDTH;
			laser.shapeBegin("RefDir"+i);
			laser.svgfile("arrow4.svg",xpos,yposzero,ARROWSIZE*1.5f,(float)(angles[i]*180/Math.PI));
			laser.shapeEnd("RefDir"+i);
		}
		for (NoteData n: notes) {
			laser.shapeBegin("Note"+n.measure);
			float ypos=Tracker.miny+(n.timestamp-(now-HISTORY))/DURATION*(Tracker.maxy-Tracker.miny);
			//logger.fine("At "+n.timestamp+", notes="+n.notes+", y="+ypos);
			for (int i=0;i<n.notes.length()&&i<4;i++) {
				if (n.notes.charAt(i) != '0') {
					float xpos=(Tracker.minx+Tracker.maxx)/2.0f - ((((float)i)/(n.notes.length()-1))-0.5f)*TICKERWIDTH;
					laser.svgfile("arrow4.svg",xpos,ypos,ARROWSIZE,(float)(angles[i]*180/Math.PI));
				}
			}
			laser.shapeEnd("Note"+n.measure);
		}
		laser.bgEnd();
	}
}

