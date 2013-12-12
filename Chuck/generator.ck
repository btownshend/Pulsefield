public class Generator  {
    int id;
    Pan2 pan;
    CCListener cclistener;
    YListener ylistener;
    PanListener panlistener;
    float ccvals[128];
    0=>int running;
    0=>int curpat;

    fun void startListeners(int id) {
		<<<"Generator.startListeners">>>;
		id=>this.id;
		cclistener.start(this,id);
		ylistener.start(this,id);
		panlistener.start(this,id);
		1=>running;
		0=>curpat;
    }

    fun void stopListeners() {
		<<<"Generator.stopListeners">>>;
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

	fun void setY(float val) {
		<<<"Generator.setY(",val,")">>>;
	}

       fun void setPattern(int patnum) {
	   if (patnum<0 || patnum>Pattern.NUMPATTERNS)
	       <<<"ID ",id," bad pattern: ",patnum>>>;
	   else
	       patnum=>curpat;
       }
    // Use Y as gain
    	fun void setY(float val) {
//	    	<<<"StkGenerator.setY(",val,")">>>;
	    pan.gain(0.2*(1.0-Math.fabs(val-0.5)*1.9));
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
	    Pattern p;
		while (running) {
		       <<<"ID ",id," waiting for measure">>>;
			BPM.measure => now;
			<<<"ID ",id," playing pattern ",curpat>>>;
			p.set(curpat);
			for (0=>int i;i<p.length;i++) {
				if (p.notes[i]!=0) {
					noteOn(p.notes[i],1.0);
				} else {
					noteOff();
				}
				if (i<p.length-1)  // Use measure event for duration of last element
				    p.durs[i]=>now;
				noteOff();
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
