import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PVector;

class ControlValues {
	PVector pos;
	int ccx, ccy;
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
	final int AbletonTrack=95;

	VisualizerDrums(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
		lastpos=new HashMap<Integer,ControlValues>();
	}

	@Override
	public void start() {
		for (int i=0;i<2;i++)
			Ableton.getInstance().arm(AbletonTrack+i,true);
	}

	@Override
	public void stop() {
		for (int i=0;i<2;i++)
			Ableton.getInstance().arm(AbletonTrack+i,false);
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
			//System.out.println("OLD="+c.ccx+","+c.ccy+", new="+ccx+","+ccy);
			c.ccx=ccx;
			c.ccy=ccy;
			boolean moving = PVector.sub(c.pos,p.origin).mag() > 0.01;
			if (c.moving != moving) {
				int cnum=p.channel%2;
				Ableton.getInstance().setControl(AbletonTrack+cnum, 0, 1, ccx);
				Ableton.getInstance().setControl(AbletonTrack+cnum, 0, 2, ccy);
				synth.play(p.id,p.channel+35,127,1,cnum);
				c.moving=moving;
			}

			c.pos=p.origin;
			curpos.put(p.id, c);
		}
		lastpos=curpos;
	}
}

