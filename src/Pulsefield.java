import oscP5.*;
import netP5.*;
import java.util.Map;

class Position {
  int x=0, y=0;
  float vx=0, vy=0;
}

class Pulsefield {
  OscP5 oscP5;
  NetAddress myRemoteLocation;
  float minx=-3.2, maxx=3.2, miny=-3.2, maxy=3.2;
  HashMap<Integer, Position> positions;


  Pulsefield() {
    /* start oscP5, listening for incoming messages from Pulsefield RealTime.m */
    oscP5 = new OscP5(this, 7002);
    oscP5.plug(this, "pfframe", "/pf/frame");
    oscP5.plug(this, "pfupdate", "/pf/update");
    oscP5.plug(this, "pfsetnpeople", "/pf/set/npeople");
    oscP5.plug(this, "pfexit", "/pf/exit");
    oscP5.plug(this, "pfentry", "/pf/entry");
    oscP5.plug(this, "pfsetminx", "/pf/set/minx");
    oscP5.plug(this, "pfsetminy", "/pf/set/miny");
    oscP5.plug(this, "pfsetmaxx", "/pf/set/maxx");
    oscP5.plug(this, "pfsetmaxy", "/pf/set/maxy");
    oscP5.plug(this, "pfstarted", "/pf/started");
    oscP5.plug(this, "pfstopped", "/pf/stopped");
    positions = new HashMap<Integer,Position>();
  }


  /* incoming osc message are forwarded to the oscEvent method. */
  void oscEvent(OscMessage theOscMessage) {
    if (theOscMessage.isPlugged() == false) {
      print("### Received an unhandled message: ");
      theOscMessage.print();
    }  /* print the address pattern and the typetag of the received OscMessage */
  }

  PVector mapposition(float x, float y) {
    return new PVector(int((x-minx)/(maxx-minx)*width), int((y-miny)/(maxy-miny)*height) );
  }

  void draw() {
  }

  void pfstarted() {
    println("PF started");
  }

  void pfstopped() {
    println("PF stopped");
  }

  void frame(int frame) {
    println("Got frame "+frame);
  }

  synchronized void pfupdate(int sampnum, float elapsed, int id, float ypos, float xpos, float yvelocity, float xvelocity, float majoraxis, float minoraxis, int groupid, int groupsize, int channel) {
    if (channel!=99) {
      print("update: ");
      print("samp="+sampnum);
      print(",elapsed="+elapsed);
      print(",id="+id);
      print(",pos=("+xpos+","+ypos+")");
      print(",vel=("+xvelocity+","+yvelocity+")");
      print(",axislength=("+majoraxis+","+minoraxis+")");
      println(",channel="+channel);
    }

    Position ps=positions.get(id);
    if (ps==null) {
      println("Unable to locate user "+id+", creating it.");
      pfentry(sampnum, elapsed, id, channel);
      ps=positions.get(id);
    }

    ps.x = (int) (( ypos-miny)*width/(maxy-miny) );
    ps.y = (int) (( xpos-minx)*height/(maxx-minx) );
  }

  // Version for simplified access (primarily for testing)
  void pfupdate(float elapsed, int id, float xpos, float ypos) {
    pf.pfupdate(0, elapsed, id, ypos*1.0/height*(maxy-miny)+miny, xpos*1.0/width*(maxx-minx)+minx, 0, 0, 0, 0, 1, 1, id);
  }

  void pfsetminx(float minx) {  
    this.minx=minx;
  }
  void pfsetminy(float miny) {  
    this.miny=miny;
  }
  void pfsetmaxx(float maxx) {  
    this.maxx=maxx;
  }
  void pfsetmaxy(float maxy) {  
    this.maxy=maxy;
  }

  void pfsetnpeople(int n) {
    println("/pf/set/npeople: now have "+n+" people");
  }

  synchronized void pfexit(int sampnum, float elapsed, int id) {
    println("exit: sampnum="+sampnum+", elapsed="+elapsed+", id="+id);
    if (positions.containsKey(id)) {
      positions.remove(id);
    } 
  }

  synchronized void pfentry(int sampnum, float elapsed, int id, int channel) {
    println("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel);
    positions.put(id, new Position());
  }
}

