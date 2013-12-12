public class Generator  {
    int id;
    Pan2 pan => JCRev rev => HPF hp => dac;
    rev.mix(0.1);
    CCListener cclistener;
    YListener ylistener;
    PanListener panlistener;
    float ccvals[128];
    0=>int running;
    0=>int curpat;

    fun void startListeners(int id) {
		<<<"Generator.startListeners">>>;
		pan.gain(0.5);   // Will change with Y positions
		rev.mix(0.1);    // Will get changed by controller
		rev.gain(0.5);   // Avoid clipping
		hp.freq(20);  // Some instruments seem to have a DC offset
		id=>this.id;
		cclistener.start(this,id);
		ylistener.start(this,id);
		panlistener.start(this,id);
		1=>running;
		Math.random2(0,Patterns.NUMPATTERNS-1)=>curpat;
    }

    fun void stopListeners() {
		<<<"Generator.stopListeners">>>;
		pan.gain(0.0);
		cclistener.stop();
		ylistener.stop();
		panlistener.stop();
		0=>running;
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

	fun void setCC2(int cc, float val) {
	    if (cc==200) {  // pattern
	        setPattern(val$int);
	    } else if (cc==201) { // reverb mix
		rev.mix(val/100.0);
	    } else {
		<<<"Bad cc: ",cc>>>;
	    }
	}

       fun void setPattern(int patnum) {
	   if (patnum<0 || patnum>Patterns.NUMPATTERNS)
	       <<<"ID ",id," bad pattern: ",patnum>>>;
	   else
	       patnum=>curpat;
       }
    // Use Y as gain
    	fun void setY(float val) {
//	    	<<<"StkGenerator.setY(",val,")">>>;
	    pan.gain(0.8*(1.0-Math.fabs(val-0.4)/0.6)+0.2);
	}

	fun void noteOn(int note, float vel) {
		<<<"noteOn() not implemented by subclass!">>>;
	}

	fun void noteOff() {
		<<<"noteOff() not implemented by subclass!">>>;
	}

	fun void stop() {
		<<<"stop() not implemented by subclass!">>>;
	}

	fun void playPattern() {
		while (running) {
		       <<<"ID ",id," waiting for measure">>>;
			BPM.measure => now;
			<<<"ID ",id," playing pattern ",curpat>>>;
			Patterns.get(curpat) @=> Pattern @p;
			for (0=>int i;i<p.length;i++) {
			    	if (!running)
				   break;
				if (p.notes[i]!=0) {
					noteOn(p.notes[i],1.0);
				} else {
					noteOff();
				}

				p.durs[i]-BPM.sixteenthNote/2=>now;
				noteOff();
				if (i<p.length-1)  // Use measure event for duration of last element			}
				    BPM.sixteenthNote/2=>now;
			}
		}
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

class YListener extends GenControlListener {
    fun void receiveEvent(OscEvent oe) {
		oe.getFloat() => float value;
		//<<<"Got yl(",value,") for ID ",id>>>;
		wrapper.setY(value);
    } 	       
    fun void start(Generator wrapper, int id) {
		start(wrapper,id,"y f");
    }
    fun void stop() {  // For some reason, the base class' stop() is not visible...
		<<<"Stopping ylistener for id ",id>>>;
		1=>done;
    }
}

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
