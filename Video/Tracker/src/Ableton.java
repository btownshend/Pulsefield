import java.util.HashMap;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

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

public class Ableton {
	static Ableton theAbleton;
	long lasttime;
	OscP5 oscP5;
	NetAddress AL;
	HashMap<Integer,Track> tracks;
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
	}

	public void sendMessage(OscMessage msg) {
		oscP5.send(msg,AL);
	}

	public void handleMessage(OscMessage msg) {
		//PApplet.println("Ableton message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (!components[1].equals("live")) 
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
}
