import processing.core.PApplet;

public class VisualizerPads extends VisualizerPS {
	Synth synth;

	VisualizerPads(PApplet parent, Synth synth) {
		super(parent);
		this.synth=synth;
	}

	@Override
	public void start() {
		super.start();
		Ableton.getInstance().setTrackSet("Pads");
	}

	@Override
	public void stop() {
		super.stop();
		synth.endallnotes();  // This doesn't seem to do anything...
	}

	@Override
	public void update(PApplet parent, People allpos) {
		super.update(parent,allpos);
		Ableton.getInstance().updateMacros(allpos);
		for (Person pos: allpos.pmap.values()) {
			//PApplet.println("ID "+pos.id+" avgspeed="+pos.avgspeed.mag());
			if (pos.isMoving())
				synth.play(pos.id,pos.channel+35,127,480,pos.channel);
		}
	}
}

