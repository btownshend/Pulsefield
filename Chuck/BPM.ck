public class BPM
{
	// global variables
	static Event @beat;
	static Event @measure;
	static dur quarterNote, eighthNote, sixteenthNote, thirtysecondNote; 

	fun void tempo(float beat) { 
		// beat argument is BPM, example 120 beats per minute
		<<<"Setting tempo to ",beat," BPM">>>;
		60.0/(beat) => float SPB; // seconds per beat 
		SPB :: second => quarterNote; 
		quarterNote*0.5 => eighthNote;
		eighthNote*0.5 => sixteenthNote;
		quarterNote*0.5 => thirtysecondNote;
	}

	// Broadcast measure-start events
	fun void broadcaster() {
		<<<"Starting BPM broadcaster">>>;
		new Event @=> beat;
		new Event @=> measure;
		while (true) {
		    <<<"Measure, quarterNote=",quarterNote>>>;
		    measure.broadcast();
		    for (0=>int i;i<4;i++) {
			beat.broadcast;
			quarterNote=>now;
		    }
		}
	}

	tempo(10);
	spork ~broadcaster();
}
