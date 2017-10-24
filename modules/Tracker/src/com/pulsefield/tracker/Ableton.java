package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.logging.Logger;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PVector;

class Clip {
	float position;
	float length;
	float loop_start;
	float loop_end;
	int state;
	private String name;
	int trackNum;
	int clipNum;
	int timesPlayed;
    private final static Logger logger = Logger.getLogger(Clip.class.getName());

	Clip(int trackNum, int clipNum) {
		this.position=0f;
		this.length=0f;
		this.loop_start=0f;
		this.loop_end=0f;
		this.state=-1;
		this.trackNum=trackNum;
		this.clipNum=clipNum;
		this.name=null;
		this.timesPlayed=0;
	}
	String getName() {
		if (name!=null)
			return name;
		// Name not set
		// Request track name from AL (not required, but makes interface cleaner)
		OscMessage msg = new OscMessage("/live/name/clip");
		msg.add(trackNum);
		msg.add(clipNum);
		Ableton.getInstance().sendMessage(msg);
		// In the mean time, fake it
		return String.format("C%d.%d",trackNum+1,clipNum+1);
	}
	void setName(String name) {
		this.name=name;
	}
	void incrementPlays() {
		timesPlayed++;
		logger.info("Clip "+trackNum+"."+clipNum+" ("+name+") has been played "+timesPlayed+" times.");
	}
}

class Track {
	static final int MAXCLIPS=60;
	private HashMap<Integer,Clip> clips;
	float meter[] = new float[2];
	String song;
	int songTrack;
	private String name;  // Use getName() to read
	Clip playing, triggered;
	int trackNum;
	int lowestSeenEmptyClip, highestSeenUsedClip;
	int infoRequestsPending;  // Incremented when we request clip info, zeroed when we get a reply
    boolean warningState;   // True when we're in a state where clip info not available
    private final static Logger logger = Logger.getLogger(Track.class.getName());

	Track(int trackNum) {
		clips=new HashMap<Integer,Clip>();
		this.name=null;
		playing=null;
		triggered=null;
		this.trackNum=trackNum;
		warningState=false;
		lowestSeenEmptyClip=MAXCLIPS;
		highestSeenUsedClip=-1;
		infoRequestsPending=0;
		clipInfoRequest();
	}

	Clip getClip(int clip) {
		Clip c=clips.get(clip);
		if (c==null) {
			c=new Clip(trackNum,clip);
			clips.put(clip,c);
		}

		return c;
	}
	void setSongTrack(String id, int songTrack) {
		this.song=id;
		this.songTrack=songTrack;
	}
	String getName() {
		if (name!=null)
			return name;
		// Name not set
		// Request track name from AL (not required, but makes interface cleaner)
		OscMessage msg = new OscMessage("/live/name/track");
		msg.add(trackNum);
		Ableton.getInstance().sendMessage(msg);
		// In the mean time, fake it
		return String.format("*%d",trackNum+1);
	}
	void setName(String name) {
		this.name=name;
	}
	void setEmptyClip(int clip) {
		infoRequestsPending=0;
		if (clip<lowestSeenEmptyClip) {
//			logger.fine("Decreasing lowest empty clip number from "+lowestSeenEmptyClip+" to "+clip);
			lowestSeenEmptyClip=clip;
		}
		clipInfoRequest();
	}
	void setOccupiedClip(int clip) {
		infoRequestsPending=0;
		if (clip>highestSeenUsedClip) {
//			logger.fine("Increasing highest seen clip number from "+highestSeenUsedClip+" to "+clip);
			highestSeenUsedClip=clip;
		}
		clipInfoRequest();
	}
	void clipInfoRequest() {
		if (highestSeenUsedClip+1<lowestSeenEmptyClip) {
//			logger.fine("Track "+trackNum+" has between "+(highestSeenUsedClip+1)+" and "+lowestSeenEmptyClip+" clips.");
			int checkClip=(int)(highestSeenUsedClip+lowestSeenEmptyClip)/2;
			Ableton.getInstance().sendMessage(new OscMessage("/live/clip/info").add(trackNum).add(checkClip));
			
			infoRequestsPending++;
			if (infoRequestsPending%100==0)
				logger.warning("Ableton not responding to clip info requests -- have "+infoRequestsPending+" requests pending");
		}
	}
	int numClips() {
		if (highestSeenUsedClip+1<lowestSeenEmptyClip) {
			// Don't yet know how many clips there are
		    if (!warningState)
			logger.warning("Ableton:numClips() - haven't determined number of clips yet for track "+this.name+"("+this.trackNum+")");
		    warningState=true;
		    clipInfoRequest();
		    return -1;
		}
		if (warningState) {
		    warningState=false;
		    logger.info("Ableton:numClips() - now have data for track "+this.name+"("+this.trackNum+")");
		}
		return lowestSeenEmptyClip;
	}
}

class TrackSet {
	String name;
	int firstTrack;
	int numTracks;
	int bgTrack;  // Track number to use for background audio
	int bgClips[];  // Array of clip numbers to use from bg track, or null to use any up to the last non-empty clip
	float tempo;
	boolean armed;
	
	TrackSet(String name, int firstTrack, int numTracks, float tempo, int bgTrack, int bgClips[]) {
		this.name=name;
		this.firstTrack=firstTrack;
		this.numTracks=numTracks;
		this.tempo = tempo;
		this.bgTrack = bgTrack;
		this.bgClips = bgClips;
		armed=false;
	}

	int getMIDIChannel(int channel) {
		//logger.fine("getMIDIChannel("+channel+")->"+(channel%numTracks));
		assert(numTracks>0);
		return channel%numTracks;
	}
	int getTrack(int channel) {
		//logger.fine("getTrack("+channel+")->"+((channel%numTracks)+firstTrack));
		assert(firstTrack>=0);
		assert(numTracks>0);
		return (channel%numTracks)+firstTrack;
	}
}


class ControlValues {
	PVector pos;
	int ccx, ccy, ccdx, ccdy, ccspeed, ccazimuth;
	boolean moving;

	ControlValues(PVector pos) {
		this.pos=pos;
		this.ccx=-1;
		this.ccy=-1;
		this.ccdx=-1;
		this.ccdy=-1;
		this.ccspeed=-1;
		this.ccazimuth=-1;
	}
}

public class Ableton {
	static Ableton theAbleton;
	long lasttime;
	OscP5 oscP5;
	NetAddress AL;
	HashMap<Integer,Track> tracks;
	HashMap<String,TrackSet> tracksets;
	TrackSet trackSet;
	HashMap<Integer,ControlValues> lastpos;  // Map from track number to current control values
	float tempo;
	float meter[] = new float[2];
	int playstate = -1;
	int bgClip = -1;   // Currently playing bg clip
    private final static Logger logger = Logger.getLogger(Ableton.class.getName());

	public void addSong(String id, String name, int firstTrack, int numTracks, float tempo, int bgTrack, int bgClips[]) {
		tracksets.put(id,new TrackSet(name,firstTrack,numTracks,tempo,bgTrack, bgClips));
		for (int i=0;i<numTracks;i++) {
			Track t=getTrack(i+firstTrack);
			t.setSongTrack(id,i);
		}
	}
	
	
	public void addSong(String id, String name, int firstTrack, int numTracks, float tempo) {
		addSong(id,name,firstTrack,numTracks,tempo,-1, null);
	}
	
	Ableton(OscP5 oscP5, NetAddress ALaddr) {
		this.oscP5=oscP5;
		lasttime=System.currentTimeMillis();
		AL=ALaddr;
		theAbleton=this;
		tracks = new HashMap<Integer,Track>();
		tempo=120;
		tracksets=new HashMap<String,TrackSet>();
		// Note that the track and clip numbers here are 1 lower than they show in AL (since the OSC interface is 0-based)
		// Format:  id, name, firstTrack, numTracks, tempo, bgTrack, bgClips[]
		addSong("QU","Quetzal",1,6,120);
		addSong("PR","Pring",8,8,108);
		addSong("OL","Oluminum",17,7,93);
		addSong("EP","Episarch",25,5,93);
		addSong("NG","New Gamelan",31,9,120);
		addSong("MB","Music Box",41,6,111);
		addSong("GA","Garage Revisited",50,6,120);
		addSong("FO","Forski",57,8,72);
		addSong("FI","Firebell",66,7,108);
		addSong("DB","Deep Blue",74,8,100);
		addSong("PB","Polybius",85,9,130);
		addSong("AN","Animals",94,4,120);
		addSong("MV","Movies",99,1,120);
		addSong("DD","DDR",101,1,120);
		addSong("Harp", "Harp",102,1,120);
		addSong("Guitar", "Guitar",103,1,120);
		addSong("Pads", "Pads",105,4,120);
		addSong("Tron", "Tron",109,1,120);
		addSong("Poly","Poly",111,3,120);
		addSong("Navier", "Navier",115,4,120);
		addSong("SteelPan","Steel Pan",119,1,120);
		addSong("Cows","Cows",		120,1,120,121,null);
		addSong("Soccer","Soccer",	122,1,120,123,null);
		addSong("Osmos","Osmos",	124,1,120,125,null);
		addSong("DNA","DNA",		 -1,0,120,125,null);
		addSong("Whack","Whack",	126,1,120);
		addSong("Bowie","Bowie",	127,1,120);
		addSong("Stickman","Stickman",-1,0,120,128,null);   
		addSong("Hunter","Hunter",	-1,0,120,129,null);  
		addSong("Freeze","Freeze",	-1,0,120,130,null);   // TODO

		lastpos=new HashMap<Integer,ControlValues>();
		trackSet=null;
		// Clear track info
		for (int songtrack=0;songtrack<8;songtrack++) {
			String fields[]={"scene","clip","track","pos"};
			for (String field: fields) {
				String path="/grid/table/"+(songtrack+1)+"/"+field;
//				logger.fine("Clearing "+path);
				OscMessage msg = new OscMessage(path);
				msg.add("-");
				TouchOSC.getInstance().sendMessage(msg);
			}
		}
	}

	public void sendMessage(OscMessage msg) {
		//logger.fine("AL<-"+msg.addrPattern());
		oscP5.send(msg,AL);
	}

	public void handleMessage(OscMessage msg) {
		//logger.fine("Ableton message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (components[1].equals("remix")) {
			if (components.length==3 && components[2].equals("error"))
				logger.warning("Got error from remix: "+msg.get(0).stringValue());
			else
				logger.warning("Unknown remix message: "+msg.toString());
		} else if (!components[1].equals("live")) 
			logger.warning("Ableton: Expected /live messages, got "+msg.toString());
		else if (components.length==3 && components[2].equals("beat")) {
			int b=msg.get(0).intValue();
			beat(b);
		} else if (components.length==3 && components[2].equals("tempo")) {
			tempo=msg.get(0).floatValue();	
			logger.fine("tempo from AL = "+tempo);
			TouchOSC.getInstance().sendMessage(new OscMessage("/tempo").add(tempo));
			logger.fine("sent to TO");
			MasterClock.settempo(tempo);
		} else if (components.length==3 && components[2].equals("play")) {
			playstate=msg.get(0).intValue();	
			logger.info("Play state changed to "+playstate);
		} else if (components.length==4 && components[2].equals("master") && components[3].equals("meter"))
			setMeter(msg.get(0).intValue(),msg.get(1).floatValue());
		else if (components.length==4 && components[2].equals("track") && components[3].equals("meter"))
			setMeter(msg.get(0).intValue(),msg.get(1).intValue(), msg.get(2).floatValue());
		else if (components.length==4 && components[2].equals("clip") && components[3].equals("position")) 
			setClipPosition(msg.get(0).intValue(),msg.get(1).intValue(),msg.get(2).floatValue(),msg.get(3).floatValue(),msg.get(4).floatValue(),msg.get(5).floatValue());
		else if (components.length==4 && components[2].equals("clip") && components[3].equals("info")) 
			setClipInfo(msg.get(0).intValue(),msg.get(1).intValue(),msg.get(2).intValue());
		else if (components.length==4 && components[2].equals("master") && components[3].equals("volume")) {
			float volume=msg.get(0).floatValue();
			TouchOSC.getInstance().sendMessage(new OscMessage("/volume").add(volume));
		} else if (components.length==4 && components[2].equals("name") && components[3].equals("clip"))
			setClipName(msg.get(0).intValue(),msg.get(1).intValue(),msg.get(2).stringValue(),msg.get(3).intValue());
		else if (components.length==4 && components[2].equals("name") && components[3].equals("track"))
			setTrackName(msg.get(0).intValue(),msg.get(1).stringValue(),msg.get(2).intValue());
		else if (components.length==4 && components[2].equals("device") && components[3].equals("param"))
			deviceParam(msg.get(0).intValue(),msg.get(1).intValue(),msg.get(2).intValue(),msg.get(3).floatValue(),msg.get(4).stringValue());
		else if (components.length==3 && components[2].equals("track"))
			; // Ignored
		else if (components.length==3 && components[2].equals("scene"))
			; // Ignored
		else if (components.length==4 && components[2].equals("clip") && components[3].equals("warping"))
			; // Ignored
		else 
			logger.warning("Unknown Ableton Message: "+msg.toString());
	}

	public void beat(float b) {
		//logger.fine("Got beat "+b+" after "+(System.currentTimeMillis()-lasttime));
		lasttime=System.currentTimeMillis();
	}

	/**
	 * Get tempo in BPM
	 * @return current tempo
	 */
	public float getTempo() {
		return tempo;
	}

	public void setALTempo(float newTempo) {
		tempo=newTempo;
		sendMessage(new OscMessage("/live/tempo").add((int)tempo));
	}
	
	public void setALVolume(float volume) {
		sendMessage(new OscMessage("/live/master/volume").add(volume));
	}
	
	public void stop() {
		sendMessage(new OscMessage("/live/stop"));
	}
	void deviceParam(int track, int device, int parameter, float value, String name) {
		//logger.fine("Track "+track+" device "+device+" parameter "+parameter+"("+name+"): "+value);
	}
	/** Handle Ableton clip info message
	 * @param track Track number
	 * @param clip Clip number
	 * @param state New state (from Ableton OSC message)
	 */
	void setClipInfo(int track, int clip, int state) {
		Track t=getTrack(track);
		if (state==0) {
//			logger.warning("Track "+track+" clip "+clip+" is empty.");
			t.setEmptyClip(clip);
			return;
		}
		t.setOccupiedClip(clip);
		Clip c = t.getClip(clip);
		logger.info("Track "+track+" ("+t.getName()+") clip "+clip+" state changed from "+c.state+" to "+state);
		int oldState=c.state;
		c.state = state;
		if (c==t.playing)
			t.playing=null;
		if (c==t.triggered)
			t.triggered=null;
		
		if (state==2)
			t.playing=c;
		else if (state==3)
			t.triggered=c;

		if (state==1 && clip==bgClip && trackSet!=null &&  track==trackSet.bgTrack && oldState!=1 && oldState!=-1) {  // For some reasons, get 1->1 messages when starting
			bgClip=-1;
			startBgTrack();   // Start a new bg track
		}
		
		OscMessage msg = new OscMessage("/grid/table/"+(t.songTrack+1)+"/track");
		if (state>=2)
			msg.add(t.getName());
		else
			msg.add("");
		TouchOSC.getInstance().sendMessage(msg);
		msg = new OscMessage("/grid/table/"+(t.songTrack+1)+"/clip");
		String m="";
		String s="";
		if (t.playing!=null) {
			m=m+t.playing.getName();
			s=s+t.playing.clipNum;
		}
		if (t.triggered!=null) {
			m=m+" -> "+t.triggered.getName();
			s=s+" -> "+t.triggered.clipNum;
		}
		msg.add(m);
		TouchOSC.getInstance().sendMessage(msg);
		msg = new OscMessage("/grid/table/"+(t.songTrack+1)+"/scene");
		msg.add(s);
		TouchOSC.getInstance().sendMessage(msg);
	}

	void setClipName(int track, int clip, String name, int color) {
		logger.fine("Track:"+track+" clip:"+clip+" name:"+name+" color:"+String.format("0x%x",color));
		Track t=getTrack(track);
		Clip c=t.getClip(clip);
		c.setName(name);
	}
	
	void setTrackName(int track, String name, int color) {
		logger.fine("Track:"+track+" name:"+name+" color:"+String.format("0x%x",color));
		Track t=getTrack(track);
		t.setName(name);
	}
	
	void setMeter(int lr, float value ) {
		if (lr==0 || lr==1) {
			meter[lr]=value;
			OscMessage msg = new OscMessage("/meter/"+(lr+1));
			msg.add(value);
			TouchOSC.getInstance().sendMessage(msg);
		} else
			logger.warning("Bad meter L/R"+lr);
	}

	Track getTrack(int track) {
		Track t=tracks.get(track);
		if (t==null) {
			t=new Track(track);
			tracks.put(track,t);
		}

		return t;
	}

	void setMeter(int track, int lr, float value ) {
		if (lr==0 || lr==1)
			getTrack(track).meter[lr]=value;
		else
			logger.warning("Bad meter L/R"+lr);
	}

	public void setClipPosition(int track, int clip, float position, float length, float loop_start, float loop_end) {
		//logger.fine("clipposition("+track+","+clip+") = "+position+"/"+length);
		Track t=getTrack(track);
		if (t==null) {
			logger.warning("Unable to get track "+track);
			return;
		}
		if (trackSet==null) {
			logger.info("Got clip position message for track "+t.getName()+"("+t.trackNum+","+track+"), but current trackSet is null; stopping clip");
			stopClip(track,clip);
		}
		else if ((t.trackNum<trackSet.firstTrack || t.trackNum>=trackSet.firstTrack+trackSet.numTracks) && trackSet.bgTrack!=t.trackNum) {
			// TODO: this can happen when a background track is playing -- should keep track of that...
			logger.info("Got clip position message for track "+t.getName()+"("+t.trackNum+","+track+"), but current trackSet, "+trackSet.name+" is for tracks "+trackSet.firstTrack+"-"+(trackSet.firstTrack+trackSet.numTracks-1)+" and bg track is "+trackSet.bgTrack+"; stopping clip");
			stopClip(track,clip);
		}
		Clip c=t.getClip(clip);
		String oldFormatted=String.format("[%.0f-%.0f]@%.0f",c.loop_start,c.loop_end,c.position);
		String newFormatted=String.format("[%.0f-%.0f]@%.0f",loop_start,loop_end,position);
		c.position=position;
		c.length=length;
		c.loop_start=loop_start;
		c.loop_end=loop_end;
		if (oldFormatted!=newFormatted) {
			// This reduces the update rate to at most 10/second
			OscMessage msg = new OscMessage("/grid/table/"+(t.songTrack+1)+"/pos");
			msg.add(newFormatted);
			TouchOSC.getInstance().sendMessage(msg);
		}
	}

	/** Retrieve a clip structure for a particular track/clip
	 * @param track Ableton track number (0-origin)
	 * @param clip Ableton clip number (0-origin)
	 * @return clip structure
	 */
	public Clip getClip(int track, int clip) {
		Track t=tracks.get(track);
		if (t==null) {
			logger.warning("getClip: track "+track+" not found");
			return null;
		}
		return t.getClip(clip);
	}

	public static Ableton getInstance() {
		return theAbleton;
	}

	public void playClip(int track, int clip) {
		logger.fine("playClip("+track+","+clip+")");
		OscMessage msg=new OscMessage("/live/play/clip");
		msg.add(track);
		msg.add(clip);
		sendMessage(msg);
		sendMessage(new OscMessage("/live/clip/info").add(track).add(clip));  // Ask for clip info
		getClip(track,clip).incrementPlays();
	}

	public void stopClip(int track, int clip) {
		OscMessage msg=new OscMessage("/live/stop/clip");
		msg.add(track);
		msg.add(clip);
		sendMessage(msg);
	}

	/** 
	 * Arm a track for MIDI (and disarm all previously armed tracks)
	 * @param track track to arm
	 * @param onOff true to arm, false to disarm
	 */
	public void setALarm(int track, boolean onOff) {
		OscMessage msg=new OscMessage("/live/arm");
		msg.add(track);
		msg.add(onOff?1:0);
		sendMessage(msg);
	}

	/** 
	 * Set a MIDI track set
	 * @param name of track set as defined in constructor
	 * @return the track set
	 */
	@SuppressWarnings("unused")
	public TrackSet setTrackSet(String name) {
		logger.info("Setting Ableton track set to "+name);
		TrackSet ts = null;
		if (name!=null) {
			ts=tracksets.get(name);
			if (ts==null) {
				logger.warning("No track set called: "+name);
			}
		}
		// No longer need to arm/disarm since we're sending OSC notes directly to the tracks
		if (false)  {
			// Disarm all other tracksets
			for (TrackSet others: tracksets.values()) {
				if (others!=ts) {
					for (int i=others.firstTrack;i<others.firstTrack+others.numTracks;i++)
						setALarm(i,false);
					others.armed=false;
				}
			}
			if (ts!=null) {
				// Arm this trackset
				for (int i=ts.firstTrack;i<ts.firstTrack+ts.numTracks;i++)
					setALarm(i,true);
				ts.armed=true;
			} 
		}
		trackSet=ts;
		if (ts!=null) {
			setALTempo(trackSet.tempo);  // Set AL tempo
			TouchOSC.getInstance().sendMessage(new OscMessage("/grid/song").add(ts.name));
			startBgTrack();
		}
		return ts;
	}
	
	public void startBgTrack() {
		if (trackSet==null) {
			logger.warning("Attempt to start bg track when trackset is null");
		}
		if (trackSet.bgTrack!=-1) {
			logger.info("Starting bg track "+trackSet.bgTrack);
			Track t=getTrack(trackSet.bgTrack);
			int[] clipChoices;
			if (t==null) {
				logger.warning("Track "+trackSet.bgTrack+" not found.");
				return;
			} else if (trackSet.bgClips != null) {
				clipChoices=trackSet.bgClips;
			} else if (t.numClips() < 0) {
				logger.warning("Num clips is not known -- starting first clip as bg");
				clipChoices=new int[1];
				clipChoices[0]=0;
			} else {
				// Use any clip on the bg track
				clipChoices=new int[t.numClips()];
				for (int i=0;i<t.numClips();i++)
					clipChoices[i]=i;
			}
			if (clipChoices.length<1)
				logger.warning("startBgTrack: Track "+trackSet.bgTrack+" has no clips");
			else {
				int minPlays=getClip(trackSet.bgTrack,clipChoices[0]).timesPlayed;
				bgClip=clipChoices[0];
				for (int i=1;i<clipChoices.length;i++) {
					int nPlays=getClip(trackSet.bgTrack,clipChoices[i]).timesPlayed;
					if (nPlays<minPlays) {
						minPlays=nPlays;
						bgClip=clipChoices[i];
					}
				}
				logger.info("Playing bg clip "+bgClip+" that has been played "+minPlays+" times (of "+clipChoices.length+" total clips)");
				playClip(trackSet.bgTrack,bgClip);
			}
		}
	}
	
	/**
	 * Set a control for a track
	 * @param track track number
	 * @param device device
	 * @param parameter parameter to set
	 * @param value value to set
	 */
	public void setALControl(int track, int device, int parameter, int value) {
		OscMessage msg=new OscMessage("/live/device");
		msg.add(track);
		msg.add(device);
		msg.add(parameter);
		msg.add(value);
		sendMessage(msg);
		//logger.fine("/live/device track="+track+", dev="+device+", param="+parameter+", value="+value);
	}

	public void updateMacros(People allpos) {
		if (trackSet==null)
			return;
		// Update internal state
		HashMap<Integer,ControlValues> curpos=new HashMap<Integer,ControlValues>();

		for (Person p: allpos.pmap.values()) {	
			int track=trackSet.getTrack(p.channel);
			if (curpos.containsKey(track)) {
				// Already have someone controlling this track; skip
				//logger.fine("Ignoring person "+p.id+" of track "+track+" since someone else is controlling it");
				continue;
			}
			ControlValues c=lastpos.get(track);
			if (c==null) {
				c=new ControlValues(p.getNormalizedPosition());
			}

			int ccx=(int)((p.getNormalizedPosition().x+1)/2*127);ccx=(ccx<0)? 0:(ccx>127?127:ccx);
			int ccy=(int)((p.getNormalizedPosition().y+1)/2*127);ccy=(ccy<0)? 0:(ccy>127?127:ccy);
			int ccdx=(int)(p.getVelocityInMeters().x*32+64); ccdx=(ccdx<0)? 0:(ccdx>127?127:ccdx);
			int ccdy=(int)(p.getVelocityInMeters().y*32+64);ccdy=(ccdy<0)? 0:(ccdy>127?127:ccdy);
			int ccspeed=(int)(p.getVelocityInMeters().mag()*64);ccspeed=(ccspeed<0)? 0:(ccspeed>127?127:ccspeed);
			// Aziumuth - goes CCW from LIDAR 1
			// speaker 1 (which is also LIDAR 1) is 0, 2 is 90, 3 (& LIDAR2) is 180, 4 is 270
			double az= p.getNormalizedPosition().heading()*180/Math.PI+90+360;
			while (az>360)
				az-=360;
			int ccazimuth=(int)(127.0/360 * az);
			//logger.fine("OLD="+c.ccx+","+c.ccy+", new="+ccx+","+ccy);

			if (ccx!=c.ccx)
				setALControl(track, 0, 1, ccx);
			if (ccy!=c.ccy)
				setALControl(track, 0, 2, ccy);
			if (ccdx!=c.ccdx)
				setALControl(track, 0, 3, ccdx);
			if (ccdy!=c.ccdy)
				setALControl(track, 0, 4, ccdy);
			if (ccspeed!=c.ccspeed)
				setALControl(track, 0, 5, ccspeed);
			if (ccazimuth!=c.ccazimuth) {
				setALControl(track, 0, 6, ccazimuth);
				//logger.fine("az="+az);
			}
			c.ccx=ccx;
			c.ccy=ccy;
			c.ccdx=ccdx;
			c.ccdy=ccdy;
			c.ccspeed=ccspeed;
			c.ccazimuth=ccazimuth;
			c.pos=p.getNormalizedPosition();
			curpos.put(track, c);
		}
		lastpos=curpos;
	}

}
