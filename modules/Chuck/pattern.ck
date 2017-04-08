public class Pattern {
	100=>int MAXPATTERN;
	int notes[MAXPATTERN];
	dur durs[MAXPATTERN];  
	0=>int length;
	fun void init() {
		set(0);
	}

	// Add a random pattern
	fun void setrandom() {
	    [0,2,4,7,9]@=>int scale[]; // Pentatonic
	    //[0,2,4,5,7,9,11]@=>int scale[]; // Major
	    57 => int basenote;
	    3 => int noctaves;
	    [  1,    2,   4,   8, 16]@=>int notelengths[];
	    [0.1,0.3,0.4,0.2,0.0]@=>float lengthprobs[];

	    16*Math.random2(1,2) => int patternlength;   // Length of pattern in 16th notes
	    0=>int total;   // Number of sixteenth notes so far
	    while (total<patternlength) {
		// note choice
		basenote+scale[Math.random2(0,scale.cap()-1)]+Math.random2(0,noctaves-1)*12 => int note;
		int notelen;
		while (true) {
		    Math.randomf() => float rnum;
		    for (0=>int i;i<lengthprobs.cap();i++)
			if (rnum<lengthprobs[i]) {
			    notelengths[i]=>notelen;
			    break;
			} else
			    rnum-lengthprobs[i]=>rnum;
		    if (notelen+total<=patternlength)
			break;
		    // else, try again -- should always add up eventually
		}
		note=>notes[length];
		notelen*BPM.sixteenthNote=>durs[length];
		length+1=>length;
		total+notelen=>total;
		//	<<<"Note ",length-1,"=",note,"/",notelen>>>;
	    }
	}
	
        fun void print() {
	    <<<"Pattern.print">>>;
	    for (0=>int i;i<length;i++) {
		<<<"Note: ",notes[i],", Dur(16th): ",durs[i]/BPM.sixteenthNote >>>;
	    }
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
