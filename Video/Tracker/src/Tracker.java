import java.awt.Color;
import java.io.IOException;
import java.util.Calendar;
import processing.core.PApplet;
import oscP5.*;
import netP5.*;
import processing.core.PVector;


public class Tracker extends PApplet {
	/**
	 * 
	 */
	private static boolean present = false;
	private static boolean autocycle = false;
	
	private static final long serialVersionUID = 1L;
	int tick=0;
	private float avgFrameRate=0;
	static OscP5 oscP5;
	NetAddress myRemoteLocation;
	static float minx=-3.61f, maxx=3.61f, miny=-3.2f, maxy=3.2f;
	static float rawminy=-3.61f, rawmaxy=3.61f, rawminx=0.63f, rawmaxx=7.04f;
	static final float screenrotation=90f;   // Rotate raw coordinates CCW by this number of degrees
	Visualizer vis[];
	VisualizerGrid visAbleton;
	VisualizerNavier visNavier;
	VisualizerDDR visDDR;
	VisualizerDot visDot;
	VisualizerChuck visChuck;
	String visnames[]={"Pads","Navier","Tron","Ableton","DDR","Poly","Voronoi","Guitar","Dot","CHucK"};
	String vispos[]={"5/1","5/2","5/3","5/4","5/5","4/1","4/2","4/3","4/4","4/5"};
	int currentvis=-1;
	static NetAddress TO, MPO, AL, MAX, CK;
	Positions positions;
	Ableton ableton;
	boolean useMAX;
	Synth synth;
	TouchOSC touchOSC;
	int mouseID;
	String configFile;
	URLConfig config;
	AutoCycler cycler;

	public void setup() {
		configFile="/Users/bst/DropBox/Pulsefield/config/urlconfig.txt";

		try {
			config=new URLConfig(configFile);
		} catch (IOException e) {
			System.err.println("Failed to open config: "+configFile+": "+e.getLocalizedMessage());
			System.exit(1);
		}

		size(1280,800, OPENGL);
		frameRate(30);
		mouseID=90;
		cycler=new AutoCycler();
		
		frame.setBackground(new Color(0,0,0));
		positions=new Positions();

		// OSC Setup (but do plugs later so everything is setup for them)
		oscP5 = new OscP5(this, config.getPort("VD"));

		TO = new NetAddress(config.getHost("TO"), config.getPort("TO"));
		MPO = new NetAddress(config.getHost("MPO"), config.getPort("MPO"));
		AL = new NetAddress(config.getHost("AL"), config.getPort("AL"));
		CK = new NetAddress(config.getHost("CK"), config.getPort("CK"));
		PApplet.println("Sending chuck commands to "+config.getHost("CK")+":"+config.getPort("CK"));
		PApplet.println("AL at "+config.getHost("AL")+":"+config.getPort("AL"));
		MAX = new NetAddress(config.getHost("MAX"), config.getPort("MAX"));
		ableton = new Ableton(oscP5, AL);
		touchOSC = new TouchOSC(oscP5, TO);
		useMAX=false;

		if (useMAX)
			synth = new Max(this,oscP5, MAX);
		else
			synth = new Synth(this);
		Scale scale=new Scale("Major","C");
		
		// Visualizers
		vis=new Visualizer[visnames.length];
		vis[0]=new VisualizerPads(this, synth);
		visNavier=new VisualizerNavier(this,synth); vis[1]=visNavier;
		vis[2]=new VisualizerTron(this,scale,synth);
		visAbleton=new VisualizerGrid(this);vis[3]=visAbleton;
		visDDR=new VisualizerDDR(this);vis[4]=visDDR;
		vis[5]=new VisualizerPoly(this,scale,synth);
		vis[6]=new VisualizerVoronoi(this,scale,synth);
		vis[7]=new VisualizerGuitar(this,synth);
		vis[8]=new VisualizerDot(this);
		vis[9]=new VisualizerChuck(this);
		setapp(9);

		// Setup OSC handlers
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
		oscP5.plug(this, "tempo", "/tempo");
		oscP5.plug(this, "ping", "/ping");
	}

	public void tempo(float t) {
		MasterClock.settempo(t);
	}

	public void ping(int code) {
		OscMessage msg = new OscMessage("/ack");
		//PApplet.println("Got ping "+code);
		msg.add(code);
		oscP5.send(msg,MPO);
	}

	public void vsetapp(OscMessage msg) {
		for (int i=0;i<vispos.length;i++) {
			if (msg.checkAddrPattern("/video/app/buttons/"+vispos[i]) ) {
				setapp(i);
				return;
			}		
		}
		println("Bad vsetup message: "+msg);
	}

	public static void sendOSC(String dest, OscMessage msg) {
		if (dest.equals("AL"))
			oscP5.send(msg,AL);
		else if (dest.equals("TO"))
			oscP5.send(msg,TO);
		else if (dest.equals("MPO"))
			oscP5.send(msg,MPO);
		else if (dest.equals("CK")) {
			oscP5.send(msg,CK);
		}
		else
			System.err.println("sendOSC: Bad destination: "+dest);
	}

	public static void sendOSC(String dest, String path, int data) {
		OscMessage msg=new OscMessage(path);
		msg.add(data);
		sendOSC(dest,msg);
	}

	synchronized public void setapp(int appNum) {
		if (appNum <0 || appNum > vis.length) {
			println("Bad video app number: "+appNum);
			return;
		}
		// Turn off old block
		for (int k=0; k<vispos.length;k++)
			if (k!=appNum) {
				OscMessage msg = new OscMessage("/video/app/buttons/"+vispos[k]);
				msg.add(0);
				sendOSC("TO",msg);
				PApplet.println("Sent "+msg.toString());
			}

		if (currentvis!=-1)
			vis[currentvis].stop();
		currentvis=appNum;
		println("Switching to app "+currentvis+": "+visnames[currentvis]);
		vis[currentvis].setName(visnames[currentvis]);
		// Turn on block for current app
		OscMessage msg = new OscMessage("/video/app/buttons/"+vispos[currentvis]);
		msg.add(1.0);
		sendOSC("TO",msg);

		msg = new OscMessage("/video/app/name");
		msg.add(visnames[currentvis]);
		sendOSC("TO",msg);

		// All notes off
		for (int ch=0;ch<16;ch++)
			synth.setCC(ch, 123, 1);
		vis[currentvis].start();
	}

	synchronized public void draw() {
		tick++;
		avgFrameRate=avgFrameRate*(1f-1f/200f)+frameRate/200f;
		if (tick%200 == 0) {
			println("Average frame rate = "+avgFrameRate);
			vis[currentvis].stats();
		}

		if (mousePressed) 
			positions.move(mouseID, mouseID%16, new PVector(mouseX*2f/width-1, mouseY*2f/height-1), mouseID, 1, tick/avgFrameRate);


		vis[currentvis].update(this, positions);
		//		translate((width-height)/2f,0);

		vis[currentvis].draw(this,positions,new PVector(width,height));
	}

	public void mouseReleased() {
		//pfexit(0, 0, 98);
		//mouseID=(mouseID-90+1)%10+90;
	}
	
	@Override
	public void keyPressed() {
		PApplet.println("Mouse key: "+key);
		if (key=='C' || key=='c') {
			mouseID=90;
			pfsetnpeople(0);
		} else if (key>='1' && key<='9')
			mouseID=90+key-'1';
		else if (key=='x'||key=='X') {
			// Current ID exits
			PApplet.println("Mouse ID "+mouseID+" exitting.");
			pfexit(0,0,mouseID);
			PApplet.println("Finished mouse exit");
		}
	}

	public static void main(String args[]) {
		if (present)
			PApplet.main(new String[] { "--present","--display=1","Tracker"});
		else
			PApplet.main(new String[] { "Tracker" });
	}

	/* incoming osc message are forwarded to the oscEvent method. */
	synchronized public void oscEvent(OscMessage theOscMessage) {
		if (theOscMessage.addrPattern().startsWith("/video/app/buttons") == true)
			vsetapp(theOscMessage);
		else if (theOscMessage.addrPattern().startsWith("/grid")) {
			visAbleton.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/live")) {
			ableton.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/navier")) {
			visNavier.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/ddr")) {
			visDDR.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/midi/pgm")) {
			synth.handleMessage(theOscMessage);
		} else if (theOscMessage.isPlugged() == false) {
			PApplet.print("### Received an unhandled message: ");
			theOscMessage.print();
		}  /* print the address pattern and the typetag of the received OscMessage */
	}

	public PVector normalizePosition(PVector pos) {
		return new PVector(pos.x*2f/(Tracker.maxx-Tracker.minx),pos.y*2f/(Tracker.maxy-Tracker.miny));
	}
	
	public PVector mapPosition(float x, float y) {
		return mapPosition(new PVector(x,y));
	}

	public PVector mapPosition(PVector raw) {
		PVector mid=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2,(Tracker.rawminy+Tracker.rawmaxy)/2);
		PVector result=PVector.sub(raw,mid);
		result.rotate((float)Math.toRadians(Tracker.screenrotation));
		// Flip y-axis since screen has origin in top left
		result.y=-result.y;
//		PApplet.println("Mapped ("+raw+") to ("+result);
		return result;
	}
	
	public void resetcoords() {
		PVector tl=mapPosition(Tracker.rawminx,Tracker.rawmaxy);
		PVector tr=mapPosition(Tracker.rawmaxx,Tracker.rawmaxy);
		PVector bl=mapPosition(Tracker.rawminx,Tracker.rawminy);
		PVector br=mapPosition(Tracker.rawmaxx,Tracker.rawminy);
		Tracker.minx=Math.min(Math.min(tl.x,tr.x),Math.min(bl.x,br.x));
		Tracker.maxx=Math.max(Math.max(tl.x,tr.x),Math.max(bl.x,br.x));
		Tracker.miny=Math.min(Math.min(tl.y,tr.y),Math.min(bl.y,br.y));
		Tracker.maxy=Math.max(Math.max(tl.y,tr.y),Math.max(bl.y,br.y));
		PApplet.println("Min/max raw:  "+Tracker.rawminx+":"+Tracker.rawmaxx+", "+Tracker.rawminy+":"+Tracker.rawmaxy);
		PApplet.println("Min/max scrn: "+Tracker.minx+":"+Tracker.maxx+", "+Tracker.miny+":"+Tracker.maxy);
	}

	synchronized public void pfstarted() {
		PApplet.println("PF started");
	}

	synchronized public void pfstopped() {
		PApplet.println("PF stopped");
		positions.clear();
	}

	void pfframe(int frame) {
		//PApplet.println("Got frame "+frame);
	}

	synchronized void add(int id, int channel) {
		positions.add(id, channel);
	}

	synchronized public void pfupdate(int sampnum, float elapsed, int id, float xpos, float ypos, float xvelocity, float yvelocity, float majoraxis, float minoraxis, int groupid, int groupsize, int channel) {
		/*	if (channel!=99) {
			PApplet.print("update: ");
			PApplet.print("samp="+sampnum);
			PApplet.print(",elapsed="+elapsed);
			PApplet.print(",id="+id);
			PApplet.print(",pos=("+xpos+","+ypos+")");
			PApplet.print(",vel=("+xvelocity+","+yvelocity+")");
			PApplet.print(",axislength=("+majoraxis+","+minoraxis+")");
			PApplet.println(",channel="+channel);
		} */
		// NOTE: Need to map xvelocity,yvelocity before using them!

		if (xpos<Tracker.rawminx) {
			PApplet.println("Got xpos ("+xpos+") less than minx ("+Tracker.rawminx+")");
		}
		if (xpos>Tracker.rawmaxx) {
			PApplet.println("Got xpos ("+xpos+") greater than maxx ("+Tracker.rawmaxx+")");
		}
		if (ypos<Tracker.rawminy) {
			PApplet.println("Got ypos ("+ypos+") less than miny ("+Tracker.rawminy+")");
		}
		if (ypos>Tracker.rawmaxy) {
			PApplet.println("Got ypos ("+ypos+") greater than maxy ("+Tracker.rawmaxy+"),");
		}

		positions.move(id, channel, normalizePosition(mapPosition(xpos, ypos)), groupid, groupsize, elapsed);
	}
	
	public void pfsetminx(float minx) {  
		Tracker.rawminx=minx;
		resetcoords();
	}
	public void pfsetminy(float miny) {  
		Tracker.rawminy=miny;
		resetcoords();
	}
	public void pfsetmaxx(float maxx) {  
		Tracker.rawmaxx=maxx;
		resetcoords();
	}
	public void pfsetmaxy(float maxy) {  
		Tracker.rawmaxy=maxy;
		resetcoords();
	}

	public void cycle() {
		if (autocycle) {
			Calendar cal=Calendar.getInstance();
			int hour=cal.get(Calendar.HOUR_OF_DAY);
			PApplet.println("Autocycling hour = "+hour);
			cycler.change(hour>=7 && hour <= 19);
		}
	}
	
	synchronized public void pfsetnpeople(int n) {
		PApplet.println("/pf/set/npeople: now have "+n+" people, size="+positions.positions.size());
		if (n==0)
			setapp(currentvis);   // Cause a reset
		if (n==0 && positions.positions.size()>0)
			cycle();
		positions.setnpeople(n);  // Also clears positions
	}

	synchronized public void pfexit(int sampnum, float elapsed, int id) {
		PApplet.println("exit: sampnum="+sampnum+", elapsed="+elapsed+", id="+id);
		positions.exit(id);
		if (positions.positions.size()==0)
			cycle();
	}

	synchronized public void pfentry(int sampnum, float elapsed, int id, int channel) {
		add(id,channel);
		PApplet.println("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel+", color="+positions.get(id).getcolor(this));
	}

	public void noteOn(int channel, int pitch, int velocity) {
		System.out.println("Got note on: channel="+channel+", pitch="+pitch+", velocity="+velocity);
	}

	public void noteOff(int channel, int pitch, int velocity) {
		System.out.println("Got note off: channel="+channel+", pitch="+pitch+", velocity="+velocity);
	}

	void controllerChange(int channel, int number, int value) {
		// Receive a controllerChange
		System.out.println("Got CC: channel="+channel+", CC="+number+", value="+value);
	}

}

