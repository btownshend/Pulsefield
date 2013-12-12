public class Pattern {
       2=>static int NUMPATTERNS;
	100=>int MAXPATTERN;
	int notes[MAXPATTERN];
	dur durs[MAXPATTERN];  
	0=>int length;

	fun void init() {
		set(0);
	}
	
	fun void set(int patnum) {
		if (patnum==0) {
			BPM.quarterNote=>dur q;
			BPM.eighthNote=>dur e;
			BPM.sixteenthNote=>dur s;
			set([60,62,64,0,60,60,0],[q,q,e,e,s,s,e]);
		} else if (patnum==1) {
			BPM.quarterNote=>dur q;
			BPM.eighthNote=>dur e;
			BPM.sixteenthNote=>dur s;
			set([60,65,67,60,61],[q,e,q,e,q]);
		}
	}

	fun void set(int n[],dur d[]) {
	    if (n.cap() != d.cap()) {
		<<<"Pattern.set with unequal length vectors: ",n.cap," and ",d.cap()>>>;
		return;
	    }
	    for (0=>int i;i<n.cap();i++) {
		n[i]=>notes[i];
		d[i]=>durs[i];
	    }
	    n.cap()=>length;
	}
}
