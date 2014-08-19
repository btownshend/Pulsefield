import java.util.HashMap;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;
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
	
	Clip(int trackNum, int clipNum) {
		this.position=0f;
		this.length=0f;
		this.loop_start=0f;
		this.loop_end=0f;
		this.state=-1;
		this.trackNum=trackNum;
		this.clipNum=clipNum;
		this.name=null;
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
	
	Track(int trackNum) {
		clips=new HashMap<Integer,Clip>();
		this.name=null;
		playing=null;
		triggered=null;
		this.trackNum=trackNum;
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
//			PApplet.println("Decreasing lowest empty clip number from "+lowestSeenEmptyClip+" to "+clip);
			lowestSeenEmptyClip=clip;
		}
		clipInfoRequest();
	}
	void setOccupiedClip(int clip) {
		infoRequestsPending=0;
		if (clip>highestSeenUsedClip) {
//			PApplet.println("Increasing highest seen clip number from "+highestSeenUsedClip+" to "+clip);
			highestSeenUsedClip=clip;
		}
		clipInfoRequest();
	}
	void clipInfoRequest() {
		if (highestSeenUsedClip+1<lowestSeenEmptyClip) {
//			PApplet.println("Track "+trackNum+" has between "+(highestSeenUsedClip+1)+" and "+lowestSeenEmptyClip+" clips.");
			int checkClip=(int)(highestSeenUsedClip+lowestSeenEmptyClip)/2;
			Ableton.getInstance().sendMessage(new OscMessage("/live/clip/info").add(trackNum).add(checkClip));
			
			infoRequestsPending++;
			if (infoRequestsPending%100==0)
				PApplet.println("Ableton not responding to clip info requests -- have "+infoRequestsPending+" requests pending");
		}
	}
	int numClips() {
		if (highestSeenUsedClip+1<lowestSeenEmptyClip) {
			// Don't yet know how many clips there are
			PApplet.println("Ableton:numClips() - haven't determined number of clips yet");
			clipInfoRequest();
			return -1;
		}
		return lowestSeenEmptyClip;
	}
}

class TrackSet {
	String name;
	int firstTrack;
	int numTracks;
	float tempo;
	boolean armed;
	
	TrackSet(String name, int firstTrack, int numTracks, float tempo) {
		this.name=name;
		this.firstTrack=firstTrack;
		this.numTracks=numTracks;
		this.tempo = tempo;
		armed=false;
	}

	int getMIDIChannel(int channel) {
		//System.out.println("getMIDIChannel("+channel+")->"+(channel%numTracks));
		return channel%numTracks;
	}
	int getTrack(int channel) {
		//System.out.println("getTrack("+channel+")->"+((channel%numTracks)+firstTrack));
		return (channel%numTracks)+firstTrack;
	}
}


class ControlValues {
	PVector pos;
	int ccx, ccy, ccdx, ccdy, ccspeed;
	boolean moving;

	ControlValues(PVector pos) {
		this.pos=pos;
		this.ccx=-1;
		this.ccy=-1;
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
	HashMap<Integer,ControlValues> lastpos;
	float tempo;
	float meter[] = new float[2];
	int playstate = -1;

	public void addSong(String id, String name, int firstTrack, int numTracks, float tempo, int nclips) {
		tracksets.put(id,new TrackSet(name,firstTrack,numTracks,tempo));
		for (int i=0;i<numTracks;i++) {
			Track t=getTrack(i+firstTrack);
			t.setSongTrack(id,i);
		}
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
		addSong("QU","Quetzal",1,6,120,60);
		addSong("PR","Pring",8,8,108,60);
		addSong("OL","Oluminum",17,7,93,60);
		addSong("EP","Episarch",25,5,93,60);
		addSong("NG","New Gamelan",31,9,120,60);
		addSong("MB","Music Box",41,8,111,60);
		addSong("GA","Garage Revisited",50,6,120,60);
		addSong("FO","Forski",57,8,72,60);
		addSong("FI","Firebell",66,7,108,60);
		addSong("DB","Deep Blue",74,8,100,60);
		addSong("AN","Animals",84,4,120,60);
		addSong("MV","Movies",89,1,120,60);
		addSong("DD","DDR",91,1,120,60);
		addSong("Harp", "Harp",92,1,120,0);
		addSong("Guitar", "Guitar",93,1,120,0);
		addSong("Pads", "Pads",95,4,120,0);
		addSong("Tron", "Tron",99,1,120,0);
		addSong("Poly","Poly",101,3,120,0);
		addSong("Navier", "Navier",105,4,120,0);
		addSong("SteelPan","Steel Pan",109,1,120,0);
		lastpos=new HashMap<Integer,ControlValues>();
		trackSet=null;
		// Clear track info
		for (int songtrack=0;songtrack<8;songtrack++) {
			String fields[]={"scene","clip","track","pos"};
			for (String field: fields) {
				String path="/grid/table/"+(songtrack+1)+"/"+field;
//				PApplet.println("Clearing "+path);
				OscMessage msg = new OscMessage(path);
				msg.add("-");
				TouchOSC.getInstance().sendMessage(msg);
			}
		}
	}

	public void sendMessage(OscMessage msg) {
		//System.out.println("AL<-"+msg.addrPattern());
		oscP5.send(msg,AL);
	}

	public void handleMessage(OscMessage msg) {
		//PApplet.println("Ableton message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (components[1].equals("remix")) {
			if (components.length==3 && components[2].equals("error"))
				PApplet.println("Got error from remix: "+msg.get(0).stringValue());
			else
				PApplet.println("Unknown remix message: "+msg.toString());
		} else if (!components[1].equals("live")) 
			PApplet.println("Ableton: Expected /live messages, got "+msg.toString());
		else if (components.length==3 && components[2].equals("beat")) {
			int b=msg.get(0).intValue();
			beat(b);
		} else if (components.length==3 && components[2].equals("tempo")) {
			tempo=msg.get(0).floatValue();	
			PApplet.println("tempo from AL = "+tempo);
			TouchOSC.getInstance().sendMessage(new OscMessage("/tempo").add(tempo));
			PApplet.println("sent to TO");
			MasterClock.settempo(tempo);
		} else if (components.length==3 && components[2].equals("play")) {
			playstate=msg.get(0).intValue();	
			PApplet.println("Play state changed to "+playstate);
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
		else 
			PApplet.println("Unknown Ableton Message: "+msg.toString());
	}

	public void beat(float b) {
		//PApplet.println("Got beat "+b+" after "+(System.currentTimeMillis()-lasttime));
		lasttime=System.currentTimeMillis();
	}

	// Get tempo in BPM
	/**
	 * @return
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
		//PApplet.println("Track "+track+" device "+device+" parameter "+parameter+"("+name+"): "+value);
	}
	/** Handle Ableton clip info message
	 * @param track Track number
	 * @param clip Clip number
	 * @param state New state (from Ableton OSC message)
	 */
	void setClipInfo(int track, int clip, int state) {
		Track t=getTrack(track);
		if (state==0) {
//			PApplet.println("Track "+track+" clip "+clip+" is empty.");
			t.setEmptyClip(clip);
			return;
		}
		t.setOccupiedClip(clip);
		Clip c = t.getClip(clip);
		PApplet.println("Track "+track+" ("+t.getName()+") clip "+clip+" state changed from "+c.state+" to "+state);
		c.state = state;
		if (c==t.playing)
			t.playing=null;
		if (c==t.triggered)
			t.triggered=null;
		
		if (state==2)
			t.playing=c;
		else if (state==3)
			t.triggered=c;

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
		PApplet.println("Track:"+track+" clip:"+clip+" name:"+name+" color:"+String.format("0x%x",color));
		Track t=getTrack(track);
		Clip c=t.getClip(clip);
		c.setName(name);
	}
	
	void setTrackName(int track, String name, int color) {
		PApplet.println("Track:"+track+" name:"+name+" color:"+String.format("0x%x",color));
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
			PApplet.println("Bad meter L/R"+lr);
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
			PApplet.println("Bad meter L/R"+lr);
	}

	public void setClipPosition(int track, int clip, float position, float length, float loop_start, float loop_end) {
		//PApplet.println("clipposition("+track+","+clip+") = "+position+"/"+length);
		Track t=getTrack(track);
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
			PApplet.println("getClip: track "+track+" not found");
			return null;
		}
		return t.getClip(clip);
	}

	public static Ableton getInstance() {
		return theAbleton;
	}

	public void playClip(int track, int clip) {
		PApplet.println("playClip("+track+","+clip+")");
		OscMessage msg=new OscMessage("/live/play/clip");
		msg.add(track);
		msg.add(clip);
		sendMessage(msg);
		sendMessage(new OscMessage("/live/clip/info").add(track).add(clip));  // Ask for clip info
	}

	public void stopClip(int track, int clip) {
		OscMessage msg=new OscMessage("/live/stop/clip");
		msg.add(track);
		msg.add(clip);
		sendMessage(msg);
	}

	/** Arm a track for MIDI (and disarm all previously armed tracks)
	 * @param track 
	 * @param onOff true to arm, false to disarm
	 */
	public void setALarm(int track, boolean onOff) {
		OscMessage msg=new OscMessage("/live/arm");
		msg.add(track);
		msg.add(onOff?1:0);
		sendMessage(msg);
	}

	/** Set a MIDI track set
	 * @param name of track set as defined in constructor
	 */
	@SuppressWarnings("unused")
	public TrackSet setTrackSet(String name) {
		PApplet.println("Setting Ableton track set to "+name);
		TrackSet ts = null;
		if (name!=null) {
			ts=tracksets.get(name);
			if (ts==null) {
				System.err.println("No track set called: "+name);
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
		}
		return ts;
	}
	
	/** Set a control for a track
	 * 
	 */
	public void setALControl(int track, int device, int parameter, int value) {
		OscMessage msg=new OscMessage("/live/device");
		msg.add(track);
		msg.add(device);
		msg.add(parameter);
		msg.add(value);
		sendMessage(msg);
		//System.out.println("/live/device track="+track+", dev="+device+", param="+parameter+", value="+value);
	}

	public void updateMacros(People allpos) {
		if (trackSet==null)
			return;
		// Update internal state
		HashMap<Integer,ControlValues> curpos=new HashMap<Integer,ControlValues>();

		for (Person p: allpos.pmap.values()) {	
			ControlValues c=lastpos.get(p.id);
			if (c==null) {
				c=new ControlValues(p.getNormalizedPosition());
			}

			int ccx=(int)((p.getNormalizedPosition().x+1)/2*127);ccx=(ccx<0)? 0:(ccx>127?127:ccx);
			int ccy=(int)((p.getNormalizedPosition().y+1)/2*127);ccy=(ccy<0)? 0:(ccy>127?127:ccy);
			int ccdx=(int)(p.getVelocityInMeters().x*32+64); ccdx=(ccdx<0)? 0:(ccdx>127?127:ccdx);
			int ccdy=(int)(p.getVelocityInMeters().y*32+64);ccdy=(ccdy<0)? 0:(ccdy>127?127:ccdy);
			int ccspeed=(int)(p.getVelocityInMeters().mag()*64);ccspeed=(ccspeed<0)? 0:(ccspeed>127?127:ccspeed);
			//System.out.println("OLD="+c.ccx+","+c.ccy+", new="+ccx+","+ccy);

			int track=trackSet.getTrack(p.channel);
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
			c.ccx=ccx;
			c.ccy=ccy;
			c.ccdx=ccdx;
			c.ccdy=ccdy;
			c.ccspeed=ccspeed;
			c.pos=p.getNormalizedPosition();
			curpos.put(p.id, c);
		}
		lastpos=curpos;
	}

}
