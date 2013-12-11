public class OSCListener { 
    string path;
    0=>int done;
    fun void listen(string msg, int port) { 
	<<<"Listening for ",msg," on port ", port>>>;
	OscRecv recv; 
	port => recv.port; 
	recv.listen(); 
	recv.event(msg) @=> OscEvent oe; 
	msg=>path;
	while (true) { 
	    oe => now; 
	    if (done)
		break;
	    while (oe.nextMsg()) { 
		receiveEvent(oe); 
	    } 
	} 
    } 
    fun void receiveEvent(OscEvent oe) { 
	<<<"Unhandled event to ",path >>>;
    } 
} 
//---- Listen on OSC for raw 
// class ListenOnRaw extends OscListener { 
//     fun void receiveEvent(OscEvent oe) { 
// 	<<< "Received raw event" >>>; 
// 	oe.getInt() => raw.freq; 
//     } 
// } 

//ListenOnRaw listenOnRaw; 
//spork ~ listenOnRaw.listenOnOsc("/leftraw/press,i", 8000); 
