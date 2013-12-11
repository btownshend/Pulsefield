10=>int MAXINSTR;
Generator @listeners[MAXINSTR];
0=>int numInstruments;

Globals gl;
<<<"port=",Globals.port>>>;
newListener nl;
delListener dl;

MidiFileIn min;
MidiMsg msg;

string filename;
if (me.args() == 0)
    me.sourceDir() + "/988-aria.mid" => filename;
 else
     me.arg(0) => filename;

int done;

while (true) {
	if (!min.open(filename)) {
		cherr <= "unable to open MIDI file: '" <= filename <= "'\n";
		me.exit();
	}

	chout <= filename <= ": " <= min.numTracks() <= " tracks\n";

	0=>done;

	for (int t; t < min.numTracks(); t++)
		//	spork ~ track(t);

	while (done < min.numTracks())
		1::second => now;

	min.close();
 }
// Not reached
nl.stop();
dl.stop();


class newListener extends OSCListener {
    Shred @listener;

    fun void receiveEvent(OscEvent oe) {
		oe.getInt() => int id;
		oe.getInt() => int type;
		<<<"Creating new instrument id ",id," of type ",type>>>;
		if (type<=2) {
			StkInstrument @instr;
			if (type==0)  {
				new BeeThree @=> instr;
			} else if (type==1) {
				new Mandolin @=> instr;
			} else if (type==2) {
				new FMVoices @=> instr;
			}
			STKGenerator @newgen;
			new STKGenerator @=> newgen;
			newgen @=> listeners[numInstruments];
			newgen.startListeners(instr,id);
		}else {
			<<<"Bad instrument type: ",type>>>;
			return;
		}
//		listeners[numInstruments].playNote(440,1.0);   // Calling playnote here hangs ChucK!, probably due to some bug around calling things from events
		numInstruments+1=>numInstruments;
		<<<"receiveEvent done">>>;
    } 	       

    spork ~listen("/chuck/new  i i",Globals.port) @=> listener;

    fun void stop() {
	listener.exit();
    }
}

class delListener extends OSCListener {
    Shred @listener;

    fun void receiveEvent(OscEvent oe) {
	oe.getInt() => int id;
	for (0=>int i;i<numInstruments;i++) {
	    if (listeners[i].id == id) {
		listeners[i].stopListeners();
		// Remove from list
		for (0=>int j;j<numInstruments-1;j++) {
		    listeners[j+1]@=>listeners[j];
		}
		<<<"Deleted instrument ",i," with ID ",id>>>;
		numInstruments-1=>numInstruments;
		i-1=>i;  // Allow this index to be retested
	    }
	}
    } 	       
    spork ~listen("/chuck/del  i",Globals.port) @=> listener;

    fun void stop() {
	listener.exit();
    }
}



fun void track(int t)
{
    while(min.read(msg, t)) {
        if(msg.when > 0::second)
            msg.when => now;
        
        if((msg.data1 & 0x90) == 0x90 && msg.data2 > 0 && msg.data3 > 0 && numInstruments>0) {
	    t%numInstruments => int inum;
	    <<<"Playing track ",t,": ",msg.data2,"/",msg.data3," on  instrument ",inum>>>;
		listeners[inum].playNote(msg.data2,msg.data3/127.0);
	}
    }
    done++;
}

