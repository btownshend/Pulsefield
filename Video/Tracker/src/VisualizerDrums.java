import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PVector;

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

public class VisualizerDrums extends VisualizerPS {
	Synth synth;
	HashMap<Integer,ControlValues> lastpos;
	TrackSet trackSet;

	VisualizerDrums(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
		lastpos=new HashMap<Integer,ControlValues>();
	}

	@Override
	public void start() {
		trackSet=Ableton.getInstance().setTrackSet("Drums");
	}

	@Override
	public void stop() {
		Ableton.getInstance().setTrackSet(null);
	}

	@Override
	public void update(PApplet parent, Positions allpos) {
		super.update(parent,allpos);
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
			System.out.println("OLD="+c.ccx+","+c.ccy+", new="+ccx+","+ccy);

			boolean moving = PVector.sub(c.pos,p.origin).mag() > 0.01;
			int track=trackSet.getTrack(p.channel);
			int cnum=trackSet.getMIDIChannel(p.channel);
			if (ccx!=c.ccx)
				Ableton.getInstance().setControl(track, 0, 1, ccx);
			if (ccy!=c.ccy)
				Ableton.getInstance().setControl(track, 0, 2, ccy);
			if (ccdx!=c.ccdx)
				Ableton.getInstance().setControl(track, 0, 3, ccdx);
			if (ccdy!=c.ccdy)
				Ableton.getInstance().setControl(track, 0, 4, ccdy);
			if (ccspeed!=c.ccspeed)
				Ableton.getInstance().setControl(track, 0, 5, ccspeed);
			c.ccx=ccx;
			c.ccy=ccy;
			c.ccdx=ccdx;
			c.ccdy=ccdy;
			c.ccspeed=ccspeed;
			if (c.moving != moving) {
				synth.play(p.id,cnum+35,127,1,cnum);
				c.moving=moving;
			}

			c.pos=p.origin;
			curpos.put(p.id, c);
		}
		lastpos=curpos;
	}
}

