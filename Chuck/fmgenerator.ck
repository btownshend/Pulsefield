public class FMGenerator extends Generator {
	SinOsc mod => SinOsc carrier;
	float moddepth;
	
	fun void setCC(int cc, float val) {
		// No way in ChucK to call super.setCC()...
		logNewCC(cc,val);

		if (cc==11) { // Modulator frequency 
			val=>mod.freq;
		} else if (cc==1) {  // Modulation depth
			val/128.0=>moddepth;
		}

		mod.gain(mod.freq()*moddepth);
	}

	fun void setFreq(float freq) {
		carrier.freq(freq);
	}

	fun void start(int id) {
		<<<"FMGenerator.start">>>;
		carrier => pan => dac;
		2=>carrier.sync;
		startListeners(id);
    }

    fun void stop() {
		<<<"FMGenerator.stop">>>;
		carrier =< pan =< dac;
		stopListeners();
	}

	fun void playNote(int note, float vel) {
//		<<<"FMGenerator.playNote(",note,",",vel,")">>>;
//		instr.freq(Std.mtof(note));
//		instr.noteOn(vel);
	}

	fun void setY(float val) {
		<<<"FMGenerator.setY(",val,")">>>;
		carrier.freq(val*880);
	}
}