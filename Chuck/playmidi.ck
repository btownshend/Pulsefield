//int MAXINSTR;
//StkInstrument @instruments[MAXINSTR];
//StkListener @listeners[MAXINSTR];
//int numInstruments;

MidiFileIn min;
MidiMsg msg;

string filename;
if (me.args() == 0)
    me.sourceDir() + "/bwv772.mid" => filename;
 else
     me.arg(0) => filename;

if (!min.open(filename)) {
    cherr <= "unable to open MIDI file: '" <= filename <= "'\n";
    me.exit();
 }

chout <= filename <= ": " <= min.numTracks() <= " tracks\n";

0=>int done;

for (int t; t < min.numTracks(); t++)
    spork ~ track(t);

while (done < min.numTracks())
    1::second => now;

min.close();

fun void track(int t)
{
    while(min.read(msg, t)) {
        if(msg.when > 0::second)
            msg.when => now;
        
        if((msg.data1 & 0x90) == 0x90 && msg.data2 > 0 && msg.data3 > 0) {
	    <<<"Playing ",msg.data2,"/",msg.data3," on ",numInstruments," instruments.">>>;
	    for (0=>int j;j<numInstruments;j++) {
		msg.data2 => Std.mtof => instruments[j].freq;
		msg.data3/127.0 => instruments[j].noteOn;
	    }
	}
    }
    done++;
}
