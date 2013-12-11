public class StkListener  {
	5 => int NVOICES;  // Polyphony, all sharing same settings
    int id;
    StkInstrument @instr[NVOICES];
	int curvoice;
    Pan2 pan;
    CCListener cc;
//    FreqListener freq;
    PanListener panlistener;
	float ccvals[128];

    fun void startListeners(StkInstrument instr, int id) {
		id=>this.id;
		for (0=>int i;i<NVOICES;i++) {
			instr@=>this.instr[i];
			this.instr[i] => pan => dac;
		}
		cc.start(this,id);
//		freq.start(this,id);
		panlistener.start(this,id);
		0=>curvoice;
    }

    fun void stopListeners() {
		for (0=>int i;i<NVOICES;i++) {
			instr[i]=<pan=<dac;
		}
		cc.stop();
//		freq.stop();
		panlistener.stop();
	}

	fun void setCC(int cc, float val) {
		if (cc<ccvals.cap()) {
			if ((val*10 $int) != (ccvals[cc]*10 $int))
				<<<"Got CC(",cc,",",val,") for ID ",id>>>;
			val=>ccvals[cc];
		}
		for (0=>int i;i<NVOICES;i++)
			instr[i].controlChange(cc,val);
	}

	fun void playNote(int note, float vel) {
		instr[curvoice].freq(Std.mtof(note));
		instr[curvoice].noteOn(vel);
		1+=>curvoice;
		if (curvoice>=NVOICES) {
			0=>curvoice;
		}
	}
}


class StkControlListener extends OSCListener {
    StkListener @wrapper;
    Shred @listener;
    int id;

    fun void start(StkListener wrapper, int id, string suffix) {
		id=>this.id;   // For debugging
		wrapper@=>this.wrapper;
		spork ~listen("/chuck/dev/"+id+"/"+suffix,Globals.port) @=> listener;
    }

    fun void stop() {
		1=>done;
    }
}

class CCListener extends StkControlListener {
    fun void receiveEvent(OscEvent oe) {
		oe.getInt() => int cc;
		oe.getFloat() => float value;
		wrapper.setCC(cc,value);
    } 	       
    fun void start(StkListener wrapper, int id) {
		start(wrapper,id,"cc i f");
    }
    fun void stop() {  // For some reason, the base class' stop() is not visible...
		1=>done;
		<<<"Stopping cclistener for id ",id>>>;
    }
}

// class FreqListener extends StkControlListener {
//     fun void receiveEvent(OscEvent oe) {
// 		oe.getFloat() => float value;
// 		<<<"Got freq(",value,") for ID ",id>>>;
// 		wrapper.instr.freq(value);
//     } 	       
//     fun void start(StkListener wrapper, int id) {
// 		start(wrapper,id,"freq f");
//     }
//     fun void stop() {  // For some reason, the base class' stop() is not visible...
// 		<<<"Stopping freqlistener for id ",id>>>;
// 		1=>done;
//     }
// }

class PanListener extends StkControlListener {
    fun void receiveEvent(OscEvent oe) {
		oe.getFloat() => float value;
		if ((wrapper.pan.pan()*10 $ int) != (value*10 $ int))
			<<<"Got pan(",value,") for ID ",id>>>;
		wrapper.pan.pan(value);
    } 	       
    fun void start(StkListener wrapper, int id) {
		start(wrapper,id,"pan f");
    }
    fun void stop() {  // For some reason, the base class' stop() is not visible...
		<<<"Stopping panlistener for id ",id>>>;
		1=>done;
    }
}
