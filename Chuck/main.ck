10=>int MAXGEN;
Generator @gens[MAXGEN];
0=>int numGens;

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
		spork ~ track(t);

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
		if (numGens>=MAXGEN) {
			<<<"Unable to create another generator, already have ",numGens>>>;
			return;
		}
		oe.getInt() => int id;
		oe.getInt() => int type;
		<<<"Creating new instrument id ",id," of type ",type>>>;
		if (type==0) {
			FMGenerator fmgen;
			fmgen @=> gens[numGens];
			fmgen.start(id);
	   } else if (type<=3) {
			StkInstrument @instr;
			if (type==1)  {
				new BeeThree @=> instr;
			} else if (type==2) {
				new Mandolin @=> instr;
			} else if (type==3) {
				new FMVoices @=> instr;
			}
			STKGenerator @newgen;
			new STKGenerator @=> newgen;
			newgen @=> gens[numGens];
			newgen.start(instr,id);
		}else {
			<<<"Bad instrument type: ",type>>>;
			return;
		}
//		gens[numGens].playNote(440,1.0);   // Calling playnote here hangs ChucK!, probably due to some bug around calling things from events
		numGens+1=>numGens;
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
	<<<"Got delete ",id," message.">>>;
	for (0=>int i;i<numGens;i++) {
//		<<<"gens[",i,"].id=",gens[i].id>>>;
	    if (gens[i].id == id) {
			gens[i].stop();
		// Remove from list
		for (i+1=>int j;j<numGens-1;j++) {
		    gens[j+1]@=>gens[j];
		}
		<<<"Deleted instrument ",i," with ID ",id>>>;
		numGens-1=>numGens;
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
	<<<"Starting MIDI reader for track ",t>>>;
    while(min.read(msg, t)) {
        if(msg.when > 0::second)
            msg.when => now;
        
        if((msg.data1 & 0x90) == 0x90 && msg.data2 > 0 && msg.data3 > 0 && numGens>0) {
	    t%numGens => int inum;
	    <<<"Playing track ",t,": ",msg.data2,"/",msg.data3," on  instrument ",inum>>>;
		gens[inum].playNote(msg.data2,msg.data3/127.0);
	}
    }
    done++;
}

