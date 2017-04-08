public class Pulsefield {
    static float maxx,maxy,minx,miny;
    0=>static int npeople;
    false=>static int running;
    7003=>static int port;
    0=>static int frame;
    0.0::second=>static dur elapsed;
    100=>static int MAXPEOPLE;
    static Person @people[MAXPEOPLE];
    static Event @changed;

    fun static void initialize() {
	maxxListener mx;
	maxyListener my;
	minxListener mnx;
	minyListener mny;
	npeopleListener np;
	stopListener stop;
	startListener start;
	pingListener ping;
	javaListener jl;
	frameListener frame;
	entryListener entry;
	exitListener exitl;
	updateListener update;
	new Event @=> changed;
    }

    // Signal a change in the Pulsefield state for any listeners
    fun static void updated() {
	changed.broadcast();
    }

    fun static void entry(int sampnum, float t, int id, int channel) {
	if (npeople==MAXPEOPLE) {
	    <<< "Hit max number of people: ",npeople,"; ignoring additions">>>;
	    return;
	}
	sampnum=>frame;
	t::second=>elapsed;
	new Person@=>Person @p;
	id=>p.id;
	channel=>p.channel;
	p@=>people[npeople];
	<<<"entry(",sampnum,",",t,",",id,",",channel,")">>>;
	1+=>npeople;
	updated();
    }

    fun static int findperson(int id) {
	for (0=>int i;i<npeople;i++)
	    if (people[i].id==id) {
		return i;
	    }
	return -1;
    }

    fun static void exit(int sampnum, float t, int id) {
	sampnum=>frame;
	t::second=>elapsed;
	findperson(id)=>int found;
	if (found==-1)
	    <<<"Person ",id," entry was missing when exitted">>>;
	else {
	    for (found=>int i;i<npeople-1;i++)
		     people[i+1]@=>people[i];
	    1-=>npeople;
	    updated();
	}
	<<<"exit(",sampnum,",",t,",",id,")">>>;
    }

    fun static void update(int sampnum, float t, int id, float x, float y, float vx, float vy, float d1, float d2, int group, int ngroup, int channel) {
	sampnum=>frame;
	t::second=>elapsed;
	findperson(id)=>int found;
	if (found==-1) {
	    <<<"Update for person not present, creating them">>>;
	    entry(sampnum,t,id,channel);
	    findperson(id)=>found;
	    if (found==-1) {
		<<<"Unable to add person">>>;
		return;
	    }
	}
	people[found]@=>Person @p;
	x=>p.x;
	y=>p.y;
	vx=>p.vx;
	vy=>p.vy;
	d1=>p.d1;
	d2=>p.d2;
	group=>p.group;
	ngroup=>p.ngroup;
	channel=>p.channel;
	<<<"update(",sampnum,",",t,",",id,",",channel,",",x,",",y,",",vx,",",vy,",",d1,",",d2,",",group,",",ngroup,",",channel,")">>>;
	updated();
    }

    fun static void setnumpeople(int n) {
	if (n != npeople) {
	    <<<"Incorrect number of people -- expected ", npeople,", but got message that there are ",n,": flushing everyone.">>>;
	    0=>npeople;
	    updated();
	}
    }
}

class entryListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
	   oe.getInt() => int sampnum;
	   oe.getFloat() => float elapsed;
	   oe.getInt() => int id;
	   oe.getInt() => int channel;
	   Pulsefield.entry(sampnum,elapsed,id,channel);
       } 	       
       spork ~listen("/pf/entry, i f i i",Pulsefield.port);
}

class exitListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
	   oe.getInt() => int sampnum;
	   oe.getFloat() => float elapsed;
	   oe.getInt() => int id;
	   Pulsefield.exit(sampnum,elapsed,id);
       } 	       
       spork ~listen("/pf/exit, i f i",Pulsefield.port);
}

class updateListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
	   oe.getInt() => int sampnum;
	   oe.getFloat() => float elapsed;
	   oe.getInt() => int id;
	   oe.getFloat() => float x;
	   oe.getFloat() => float y;
	   oe.getFloat() => float vx;
	   oe.getFloat() => float vy;
	   oe.getFloat() => float d1;
	   oe.getFloat() => float d2;
	   oe.getInt() => int group;
	   oe.getInt() => int ngroup;
	   oe.getInt() => int channel;
	   Pulsefield.update(sampnum,elapsed,id,x,y,vx,vy,d1,d2,group,ngroup,channel);
       } 	       
       spork ~listen("/pf/update, i f i f f f f f f i i i",Pulsefield.port);
}

class stopListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       false => Pulsefield.running;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/stopped",Pulsefield.port);
}
    
class startListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       true => Pulsefield.running;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/started",Pulsefield.port);
}
    
class frameListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getInt() => Pulsefield.frame;
       } 	       
       spork ~listen("/pf/frame i",Pulsefield.port);
}

class maxxListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getFloat() => Pulsefield.maxx;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/set/maxx f",Pulsefield.port);
}

class maxyListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getFloat() => Pulsefield.maxy;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/set/maxy f",Pulsefield.port);
}

class minxListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getFloat() => Pulsefield.minx;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/set/minx f",Pulsefield.port);
}

class minyListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getFloat() => Pulsefield.miny;
	       Pulsefield.updated();
       } 	       
       spork ~listen("/pf/set/miny f",Pulsefield.port);
}

class npeopleListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
	   Pulsefield.setnumpeople(oe.getInt());
       } 	       
       spork ~listen("/pf/set/npeople i",Pulsefield.port);
}

class pingListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getInt() => int code;
	       <<<"got ping(",code,")">>>;
       } 	       
       spork ~listen("/ping i",Pulsefield.port);
}

class javaListener extends OSCListener {
       fun void receiveEvent(OscEvent oe) {
    	       oe.getFloat() => float code;
	       <<<"got chuck(",code,")">>>;
       } 	       
       spork ~listen("/chuck/update f",Pulsefield.port);
}
