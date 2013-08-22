import processing.core.PApplet;

public class VisualizerDrums extends VisualizerPS {
	Synth synth;

	VisualizerDrums(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
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
		Ableton.getInstance().updateMacros(allpos);
		TrackSet ts=Ableton.getInstance().trackSet;
		if (ts==null)
			return;
		for (Position pos: allpos.positions.values()) {
			//PApplet.println("ID "+pos.id+" avgspeed="+pos.avgspeed.mag());
			if (pos.avgspeed.mag() > 0.1) {
				int cnum=ts.getMIDIChannel(pos.channel);
				synth.play(pos.id,cnum+35,127,480,cnum);
			}
		}
	}
}

