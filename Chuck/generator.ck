public class Generator  {
    int id;
    Pan2 pan;
    CCListener cclistener;
//    FreqListener freq;
    PanListener panlistener;
	float ccvals[128];

    fun void startListeners(int id) {
		<<<"Generator.startListeners">>>;
		id=>this.id;
		cclistener.start(this,id);
//		freq.start(this,id);
		panlistener.start(this,id);
    }

    fun void stopListeners() {
		<<<"Generator.stopListeners">>>;
		cclistener.stop();
//		freq.stop();
		panlistener.stop();
	}

	fun void logNewCC(int cc, float val) {
		if (cc<ccvals.cap()) {
			if ((val $int) != (ccvals[cc] $int))
			<<<"Got CC(",cc,",",val,") for ID ",id>>>;
			val=>ccvals[cc];
		}
	}
	
	fun void setCC(int cc, float val) {
		<<<"Generator.setCC(",cc,",",val,")">>>;
		logNewCC(cc,val);
	}

	fun void playNote(int note, float vel) {
		<<<"playNote() not implemented by subclass!">>>;
	}

	fun void stop() {
		<<<"stop() not implemented by subclass!">>>;
	}
}


class GenControlListener extends OSCListener {
    Generator @wrapper;
    Shred @listener;
    int id;

    fun void start(Generator wrapper, int id, string suffix) {
		id=>this.id;   // For debugging
		wrapper@=>this.wrapper;
		spork ~listen("/chuck/dev/"+id+"/"+suffix,Globals.port) @=> listener;
    }

    fun void stop() {
		1=>done;
    }
}

class CCListener extends GenControlListener {
    fun void receiveEvent(OscEvent oe) {
		oe.getInt() => int cc;
		oe.getFloat() => float value;
		wrapper.setCC(cc,value);
    } 	       
    fun void start(Generator wrapper, int id) {
		start(wrapper,id,"cc i f");
    }
    fun void stop() {  // For some reason, the base class' stop() is not visible...
		1=>done;
		<<<"Stopping cclistener for id ",id>>>;
    }
}

// class FreqListener extends GenControlListener {
//     fun void receiveEvent(OscEvent oe) {
// 		oe.getFloat() => float value;
// 		<<<"Got freq(",value,") for ID ",id>>>;
// 		wrapper.instr.freq(value);
//     } 	       
//     fun void start(Generator wrapper, int id) {
// 		start(wrapper,id,"freq f");
//     }
//     fun void stop() {  // For some reason, the base class' stop() is not visible...
// 		<<<"Stopping freqlistener for id ",id>>>;
// 		1=>done;
//     }
// }

class PanListener extends GenControlListener {
    fun void receiveEvent(OscEvent oe) {
		oe.getFloat() => float value;
		if ((wrapper.pan.pan()*10 $ int) != (value*10 $ int))
			<<<"Got pan(",value,") for ID ",id>>>;
		wrapper.pan.pan(value);
    } 	       
    fun void start(Generator wrapper, int id) {
		start(wrapper,id,"pan f");
    }
    fun void stop() {  // For some reason, the base class' stop() is not visible...
		<<<"Stopping panlistener for id ",id>>>;
		1=>done;
    }
}
