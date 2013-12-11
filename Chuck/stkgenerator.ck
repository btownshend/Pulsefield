public class STKGenerator extends Generator {
    StkInstrument @instr;

	fun void setCC(int cc, float val) {
		instr.controlChange(cc,val);
		// No way in ChucK to call super.setCC()...
		logNewCC(cc,val);
	}

	fun void start(StkInstrument newinstr, int id) {
		<<<"STKGenerator.start">>>;
		newinstr@=>instr;
		instr => pan => dac;
		startListeners(id);
    }

    fun void stop() {
		<<<"STKGenerator.stop">>>;
		instr=<pan=<dac;
		stopListeners();
	}

	fun void playNote(int note, float vel) {
//		<<<"STKGenerator.playNote(",note,",",vel,")">>>;
		instr.freq(Std.mtof(note));
		instr.noteOn(vel);
	}
}