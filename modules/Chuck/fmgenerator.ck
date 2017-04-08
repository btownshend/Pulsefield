public class FMGenerator extends Generator {
	SinOsc mod => SinOsc carrier => ADSR env;
	float moddepth;
	
	fun void setCC(int cc, float val) {
		// No way in ChucK to call super.setCC()...
		logNewCC(cc,val);

		if (cc==11) { // Modulator frequency 
			val=>mod.freq;
		} else if (cc==1) {  // Modulation depth
			val=>moddepth;
		} else {
		     setCC2(cc,val);
		     }
		mod.gain(moddepth);
	}

	fun void setFreq(float freq) {
		carrier.freq(freq);
	}

	fun void start(int id) {
		<<<"FMGenerator.start">>>;
		pan.gain(0.5);  // Avoid clipping
		env => rev;
		2=>carrier.sync;
		startListeners(id);
    }

    fun void stop() {
		<<<"FMGenerator.stop">>>;
		env =< rev;
		stopListeners();
	}

	fun void playNote(int note, float vel) {
//		<<<"FMGenerator.playNote(",note,",",vel,")">>>;
//		instr.freq(Std.mtof(note));
//		instr.noteOn(vel);
	}

	fun void setY(float val) {
//		<<<"FMGenerator.setY(",val,")">>>;
		carrier.freq(Math.pow(2.0,val*4)*110);
	}

	fun void noteOn(int note, float vel) {
		env.keyOn();
	}

	fun void noteOff() {
		env.keyOff();
	}

    
}