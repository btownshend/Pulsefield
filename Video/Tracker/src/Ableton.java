import java.util.HashMap;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;
import processing.core.PVector;

class Clip {
	float position;
	float length;
	int state;

	Clip() {
		this.position=0f;
		this.length=0f;
		this.state=-1;
	}
}

class Track {
	HashMap<Integer,Clip> clips;
	float meter[] = new float[2];

	Track() {
		clips=new HashMap<Integer,Clip>();
	}

	Clip getClip(int clip) {
		Clip c=clips.get(clip);
		if (c==null) {
			c=new Clip();
			clips.put(clip,c);
		}

		return c;
	}

}

class TrackSet {
	String name;
	int firstTrack;
	int numTracks;
	boolean armed;
	TrackSet(String name, int firstTrack, int numTracks) {
		this.name=name;
		this.firstTrack=firstTrack;
		this.numTracks=numTracks;
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

	Ableton(OscP5 oscP5, NetAddress ALaddr) {
		this.oscP5=oscP5;
		lasttime=System.currentTimeMillis();
		AL=ALaddr;
		theAbleton=this;
		tracks = new HashMap<Integer,Track>();
		tempo=120;
		tracksets=new HashMap<String,TrackSet>();
		tracksets.put("Harp", new TrackSet("Harp",92,1));
		tracksets.put("Guitar", new TrackSet("Guitar",93,1));
		tracksets.put("Pads", new TrackSet("Pads",95,4));
		tracksets.put("Tron", new TrackSet("Tron",99,1));
		tracksets.put("Poly",new TrackSet("Poly",100,1));
		tracksets.put("Navier", new TrackSet("Navier",101,4));
		lastpos=new HashMap<Integer,ControlValues>();
		trackSet=null;
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
			System.err.println("/remix:"+ msg.toString());
			System.exit(1);
		} else if (!components[1].equals("live")) 
			PApplet.println("Ableton: Expected /live messages, got "+msg.toString());
		else if (components.length==3 && components[2].equals("beat")) {
			int b=msg.get(0).intValue();
			beat(b);
		} else if (components.length==3 && components[2].equals("tempo")) 
			tempo=msg.get(0).floatValue();	
		else if (components.length==3 && components[2].equals("play")) {
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
		else if (components.length==4 && components[2].equals("master") && components[3].equals("volume"))
			// Track-meter
			;
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

	void deviceParam(int track, int device, int parameter, float value, String name) {
		PApplet.println("Track "+track+" device "+device+" parameter "+parameter+"("+name+"): "+value);
	}
	/** Handle Ableton clip info message
	 * @param track Track number
	 * @param clip Clip number
	 * @param state New state (from Ableton OSC message)
	 */
	void setClipInfo(int track, int clip, int state) {
		Track t=getTrack(track);
		Clip c = t.getClip(clip);
		PApplet.println("Track "+track+" clip "+clip+" state changed from "+c.state+" to "+state);
		c.state = state;
	}

	void setMeter(int lr, float value ) {
		if (lr==0 || lr==1)
			meter[lr]=value;
		else
			PApplet.println("Bad meter L/R"+lr);
	}

	Track getTrack(int track) {
		Track t=tracks.get(track);
		if (t==null) {
			t=new Track();
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
		c.position=position;
		c.length=length;
	}

	/** Retrieve a clip structure for a particular track/clip
	 * @param track Ableton track number (0-origin)
	 * @param clip Ableton clip number (0-origin)
	 * @return clip structure
	 */
	public Clip getClip(int track, int clip) {
		Track t=tracks.get(track);
		if (t==null)
			return null;
		return t.clips.get(clip);
	}

	public static Ableton getInstance() {
		return theAbleton;
	}

	public void playClip(int track, int clip) {
		OscMessage msg=new OscMessage("/live/play/clip");
		msg.add(track);
		msg.add(clip);
		sendMessage(msg);
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
	public void arm(int track, boolean onOff) {
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
						arm(i,false);
					others.armed=false;
				}
			}
			if (ts!=null) {
				// Arm this trackset
				for (int i=ts.firstTrack;i<ts.firstTrack+ts.numTracks;i++)
					arm(i,true);
				ts.armed=true;
			} 
		}
		trackSet=ts;
		return ts;
	}
	
	/** Set a control for a track
	 * 
	 */
	public void setControl(int track, int device, int parameter, int value) {
		OscMessage msg=new OscMessage("/live/device");
		msg.add(track);
		msg.add(device);
		msg.add(parameter);
		msg.add(value);
		sendMessage(msg);
		System.out.println("/live/device track="+track+", dev="+device+", param="+parameter+", value="+value);
	}

	public void updateMacros(Positions allpos) {
		if (trackSet==null)
			return;
		// Update internal state
		HashMap<Integer,ControlValues> curpos=new HashMap<Integer,ControlValues>();

		for (Position p: allpos.positions.values()) {	
			ControlValues c=lastpos.get(p.id);
			if (c==null) {
				c=new ControlValues(p.origin);
			}

			int ccx=(int)((p.origin.x+1)/2*127);
			int ccy=(int)((p.origin.y+1)/2*127);
			int ccdx=(int)((p.avgspeed.x*3+1)/2*127); ccdx=(ccdx<0)? 0:(ccdx>127?127:ccdx);
			int ccdy=(int)((p.avgspeed.y*3+1)/2*127);ccdy=(ccdy<0)? 0:(ccdy>127?127:ccdy);
			int ccspeed=(int)(p.avgspeed.mag()*2*127);ccspeed=(ccspeed<0)? 0:(ccspeed>127?127:ccspeed);
			//System.out.println("OLD="+c.ccx+","+c.ccy+", new="+ccx+","+ccy);

			int track=trackSet.getTrack(p.channel);
			if (ccx!=c.ccx)
				setControl(track, 0, 1, ccx);
			if (ccy!=c.ccy)
				setControl(track, 0, 2, ccy);
			if (ccdx!=c.ccdx)
				setControl(track, 0, 3, ccdx);
			if (ccdy!=c.ccdy)
				setControl(track, 0, 4, ccdy);
			if (ccspeed!=c.ccspeed)
				setControl(track, 0, 5, ccspeed);
			c.ccx=ccx;
			c.ccy=ccy;
			c.ccdx=ccdx;
			c.ccdy=ccdy;
			c.ccspeed=ccspeed;
			c.pos=p.origin;
			curpos.put(p.id, c);
		}
		lastpos=curpos;
	}

}
