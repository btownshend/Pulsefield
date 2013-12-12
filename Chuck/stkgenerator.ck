public class STKGenerator extends Generator {
    StkInstrument @instr;

	fun void setCC(int cc, float val) {
	    // No way in ChucK to call super.setCC()...
	    logNewCC(cc,val);

	    if (cc<=128 || cc==1071)
		instr.controlChange(cc,val);
	    else
		setCC2(cc,val);
	}

	fun void start(StkInstrument newinstr, int id) {
		<<<"STKGenerator.start">>>;
		newinstr@=>instr;
		pan.gain(0.2);  // Avoid clipping
		instr => pan;
		startListeners(id);
    }

    fun void stop() {
		<<<"STKGenerator.stop">>>;
	        instr=<pan;
		stopListeners();
	}

	fun void noteOn(int note, float vel) {
		<<<"STKGenerator.noteOn(",note,",",vel,")">>>;
		instr.freq(Std.mtof(note));
		instr.noteOn(vel);
	}

	fun void noteOff() {
		instr.noteOff(1.0);
	}
}
