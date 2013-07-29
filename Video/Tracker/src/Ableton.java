import java.util.HashMap;

import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import processing.core.PApplet;

class Clip {
	float position;
	float length;
	Clip() {
		this.position=0f;
		this.length=0f;
	}
}

class Track {
	HashMap<Integer,Clip> clips;
	Track() {
		clips=new HashMap<Integer,Clip>();
	}
}

public class Ableton {
	static Ableton theAbleton;
	long lasttime;
	OscP5 oscP5;
	NetAddress AL;
	HashMap<Integer,Track> tracks;
	float tempo;
	
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
		} else if (components.length==3 && components[2].equals("tempo")) {
			tempo=msg.get(0).floatValue();	
		} else if (components.length==4 && components[2].equals("clip") && components[3].equals("position")) {
			setClipPosition(msg.get(0).intValue(),msg.get(1).intValue(),msg.get(2).floatValue(),msg.get(3).floatValue(),msg.get(4).floatValue(),msg.get(5).floatValue());
		} else 
			PApplet.println("Unknown Ableton Message: "+msg.toString());
	}

	public void beat(int b) {
		PApplet.println("Got beat "+b+" after "+(System.currentTimeMillis()-lasttime));
		lasttime=System.currentTimeMillis();
	}

	// Get tempo in BPM
	public float getTempo() {
		return tempo;
	}
	
	public void setClipPosition(int track, int clip, float position, float length, float loop_start, float loop_end) {
		//PApplet.println("clipposition("+track+","+clip+") = "+position+"/"+length);
		Track t=tracks.get(track);
		if (t==null) {
			t=new Track();
			tracks.put(track,t);
		}
		Clip c=t.clips.get(clip);
		if (c==null) {
			c=new Clip();
			t.clips.put(clip,c);
		}
		c.position=position;
		c.length=length;
	}

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
