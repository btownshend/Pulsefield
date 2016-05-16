import java.awt.Color;
import java.io.IOException;

import codeanticode.syphon.SyphonServer;
import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import oscP5.OscProperties;
import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;


public class Tracker extends PApplet {
	/**
	 * 
	 */
	public static final String SVGDIRECTORY="../../../SVGFiles/";
	private static boolean present = false;
	private static boolean autocycle = true;
	private static boolean starting = true;   // Disable bad OSC messages before setup
	private static boolean genFrameMsgs = false;
	private static final long serialVersionUID = 1L;
	int tick=0;
	private float avgFrameRate=0;
	static OscP5 oscP5;
	NetAddress myRemoteLocation;
	static float minx=-5f, maxx=5f, miny=0f, maxy=5f;
	static float rawminy=0f, rawmaxy=5f, rawminx=-5f, rawmaxx=5f;
	static final float screenrotation=0f; // 90f;   // Rotate raw coordinates CCW by this number of degrees
	Visualizer vis[];
	VisualizerGrid visAbleton;
	VisualizerProximity visProx;
	VisualizerNavier visNavier;
	VisualizerDDR visDDR;
	VisualizerDot visDot;
	VisualizerChuck visChuck;
	VisualizerMenu visMenu;
	public static final String visnames[]={"Pads","Navier","Tron","Grid","DDR","Poly","Voronoi","Guitar","Dot","CHucK","Proximity","Cows","Soccer","Menu","Visualizer"};
	public static boolean selectable[]={false,true,true,true,true,true,false,true,false,false,true,true,true,false,true};
//	public static boolean selectable[]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,true};
	String vispos[]={"5/1","5/2","5/3","5/4","5/5","4/1","4/2","4/3","4/4","4/5","3/1","3/2","3/3","3/4","3/5"};
	int currentvis=-1;
	static NetAddress TO, MPO, AL, MAX, CK, VD;
	People people, mousePeople;
	Ableton ableton;
	boolean useMAX;
	Synth synth;
	TouchOSC touchOSC;
	int mouseID;
	String configFile;
	URLConfig config;
	AutoCycler cycler;
	PVector prevMousePos;
	Boolean prevMousePressed;
	PVector mouseVel;  // Average mouse velocity
	int lastFrameReceived=3;
	Fourier fourier;
	SyphonServer server=null;
	String renderer=P2D;
	Boolean useSyphon = false;
	PGraphics canvas;
	
	public void settings() {
		// If Tracker uses FX2D or P2D for renderer, then we can't do 3D and vortexRenderer will be blank!
		size(1280, 720, renderer);
		//pixelDensity(2);  // This breaks the Navier visualizer
	}

	public void setup() {
		configFile="/Users/bst/DropBox/Pulsefield/src/urlconfig.txt";

		try {
			config=new URLConfig(configFile);
		} catch (IOException e) {
			System.err.println("Failed to open config: "+configFile+": "+e.getLocalizedMessage());
			System.exit(1);
		}

		frameRate(30);
		mouseID=90;
		cycler=new AutoCycler();
		
		frame.setBackground(new Color(0,0,0));
		people=new People();
		mousePeople=new People();
		
		// OSC Setup (but do plugs later so everything is setup for them)
		OscProperties oscProps = new OscProperties();
		oscProps.setDatagramSize(10000);  // Increase datagram size to handle incoming blobs
		oscProps.setListeningPort(config.getPort("VD"));
		oscP5 = new OscP5(this, oscProps);

		TO = new NetAddress(config.getHost("TO"), config.getPort("TO"));
		MPO = new NetAddress(config.getHost("MPO"), config.getPort("MPO"));
		AL = new NetAddress(config.getHost("AL"), config.getPort("AL"));
		CK = new NetAddress(config.getHost("CK"), config.getPort("CK"));
		VD = new NetAddress(config.getHost("VD"), config.getPort("VD"));
		PApplet.println("Sending to mayself at "+VD.address()+":"+VD.port());
		PApplet.println("Sending chuck commands to "+config.getHost("CK")+":"+config.getPort("CK"));
		PApplet.println("AL at "+config.getHost("AL")+":"+config.getPort("AL"));
		MAX = new NetAddress(config.getHost("MAX"), config.getPort("MAX"));
		touchOSC = new TouchOSC(oscP5, TO);
		ableton = new Ableton(oscP5, AL);

		new Laser(oscP5, new NetAddress(config.getHost("LASER"), config.getPort("LASER")));
		synth = new Max(this,oscP5, MAX);

		synth.play(0,64,100,100,1);
		Scale scale=new Scale("Major","C");
		
		prevMousePos=new PVector(0f,0f);
		prevMousePressed=false;
		mouseVel=new PVector(0f,0f);
		
		// Facility for real-time FFT of default input
		fourier=new Fourier(this);
		
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
		vis[10]=new VisualizerProximity(this);
		vis[11]=new VisualizerCows(this);
		vis[12]=new VisualizerSoccer(this);
		visMenu=new VisualizerMenu(this);vis[13]=visMenu;

		vis[14]=new VisualizerMinim(this,fourier,renderer!=FX2D);
		setapp(12);
		
		// Setup OSC handlers
		oscP5.plug(this, "pfframe", "/pf/frame");
		oscP5.plug(this, "pfupdate", "/pf/update");
		oscP5.plug(this, "pfbackground","/pf/background");
		oscP5.plug(this, "pfgeo","/pf/geo");
		oscP5.plug(this, "pfgroup", "/pf/group");
		oscP5.plug(this, "pfleg", "/pf/leg");
		oscP5.plug(this, "pfbody", "/pf/body");
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
		oscP5.plug(this, "volume", "/volume");
		oscP5.plug(this, "ping", "/ping");
		oscP5.plug(visAbleton,  "songIncr", "/touchosc/song/incr");
		
		canvas = this.createGraphics(width/2, height/2);
		PApplet.println("Setup complete");
		starting = false;
	}

	public void tempo(float t) {
		PApplet.println("tempo("+t+")");
		MasterClock.settempo(t);
		Ableton.getInstance().setALTempo(t);
	}

	public void volume(float vol) {
		Ableton.getInstance().setALVolume(vol);
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
		else if (dest.equals("CK"))
			oscP5.send(msg,CK);
		else if (dest.equals("Laser"))
			Laser.getInstance().sendMessage(msg);
		else if (dest.equals("VD"))
			oscP5.send(msg,VD);
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
		vis[currentvis].start();
	}

	synchronized public void draw() {
		tick++;
		avgFrameRate=avgFrameRate*(1f-1f/200f)+frameRate/200f;
		if (tick%200 == 0) {
			println("Average frame rate = "+avgFrameRate);
			vis[currentvis].stats();
		}

		if (mousePressed) {
			Person p=mousePeople.getOrCreate(mouseID,mouseID%16);
			PVector mousePos=normalizedToFloor(new PVector(mouseX*2f/width-1, mouseY*2f/height-1));
			if (prevMousePressed) {
				mouseVel.mult(0.9f);
				mouseVel.add(PVector.mult(PVector.sub(mousePos, prevMousePos),0.1f*frameRate));
			}
			p.move(mousePos, mouseVel, mouseID, 1, tick/avgFrameRate);
			Leg legs[]=p.legs;
			legs[0].move(PVector.add(mousePos,new PVector(0.0f,-0.15f)),mouseVel);
			legs[1].move(PVector.add(mousePos,new PVector(0.0f,0.15f)), mouseVel);
			prevMousePos=mousePos;
			// Additional settings for sending OSC messages
			p.diam=0.15f;
			p.sep=0.3f;
			p.groupid=p.id;
			p.groupsize=1;
//			PApplet.println("Moved mouse ID "+mouseID+" to "+mousePos+" with velocity "+p.getVelocityInMeters());
		} else {
			mouseVel.set(0f,0f);
		}
		prevMousePressed=mousePressed;
		if (visMenu.hotSpotCheck(this,people))
			setapp(13);

		sendMouseOSC();

		vis[currentvis].update(this, people);
		//		translate((width-height)/2f,0);

		canvas.beginDraw();
		vis[currentvis].draw(this, canvas,people,new PVector(canvas.width,canvas.height));
		canvas.endDraw();
		this.image(canvas,0,0,width, height);

		vis[currentvis].drawLaser(this,people);

		// Syphon setup, requires OpenGL renderer (not FX2D?)
		// Currently seems to break display
		if (renderer != FX2D && useSyphon && server==null)
			server = new SyphonServer(this, "Tracker");
		
		if (server != null) {
			beginPGL();
			server.sendScreen();
			endPGL();
		}
		SyphonTest.draw(this);
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
			mousePeople.pmap.clear();
			clearMice();
		} else if (key>='1' && key<='9')
			mouseID=90+key-'1';
		else if (key=='x'||key=='X') {
			// Current ID exits
			PApplet.println("Mouse ID "+mouseID+" exitting.");
			mousePeople.pmap.remove(mouseID);
			clearMice();
		} else if (key=='a' || key=='A') {
			// Advance to next app
			cycle();
		}
	}

	public static void main(String args[]) {
		if (present)
			PApplet.main(new String[] { "--present","--display=1","Tracker"});
		else
			PApplet.main(new String[] {"--display=2","Tracker" });
	}

	/* incoming osc message are forwarded to the oscEvent method. */
	synchronized public void oscEvent(OscMessage theOscMessage) {
		if (starting)
			return;
		if (theOscMessage.isPlugged() == true) 
			; // Handled elsewhere
		else if (theOscMessage.addrPattern().startsWith("/video/app/buttons") == true)
			vsetapp(theOscMessage);
		else if (theOscMessage.addrPattern().startsWith("/grid")) {
			visAbleton.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/live") || theOscMessage.addrPattern().startsWith("/remix/error")) {
			ableton.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/navier")) {
			visNavier.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/ddr")) {
			visDDR.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/midi/pgm")) {
			synth.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/pf/set")) {
			// PApplet.println("Unhandled set message: "+theOscMessage.addrPattern());
		} else if (theOscMessage.addrPattern().startsWith("/pf/background")) {
			// PApplet.println("Unhandled set message: "+theOscMessage.addrPattern());
		} else if (theOscMessage.addrPattern().startsWith("/vis/")) {
			// PApplet.println("Unhandled vis message: "+theOscMessage.addrPattern());
		} else {
			PApplet.print("### Received an unhandled message: ");
			theOscMessage.print();
		}  /* print the address pattern and the typetag of the received OscMessage */
	}

	public static PVector mapVelocity(PVector velInMetersPerSecond) {
		return new PVector(-velInMetersPerSecond.x*2f/(Tracker.maxx-Tracker.minx),velInMetersPerSecond.y*2f/(Tracker.maxy-Tracker.miny));
	}
	
	public static PVector floorToNormalized(float x, float y, boolean preserveAspect) {
		return floorToNormalized(new PVector(x,y),preserveAspect);
	}

	public static PVector floorToNormalized(float x, float y) {
		return floorToNormalized(new PVector(x,y),false);
	}

	// Map position in meters to normalized position where (minx,miny) maps to (-1,1) and (max,maxy) maps to (1,-1)
	// (flipped y-coord for screen use)
	public static PVector floorToNormalized(PVector raw, boolean preserveAspect) {
		PVector mid=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2,(Tracker.rawminy+Tracker.rawmaxy)/2);
		PVector result=PVector.sub(raw,mid);
		result.rotate((float)Math.toRadians(Tracker.screenrotation));
		// Flip y-axis since screen has origin in top left
	//	result.y=-result.y;
		result.x=-result.x;
		if (preserveAspect)
			result=PVector.mult(result,2f/Math.min(maxx-minx,maxy-miny));
		else
			result.set(result.x*2f/(Tracker.maxx-Tracker.minx),result.y*2f/(Tracker.maxy-Tracker.miny));
	
//		PApplet.println("Mapped ("+raw+") to ("+result);
		return result;
	}
	public static PVector floorToNormalized(PVector raw) {
		return floorToNormalized(raw,false);
	}
	
	/* Not currently used... TODO
	// Convert from floor coordinate (in meters) to window position in given window size
	// Keep scaling same in x and y direction, so may result in less than full utilization of window
	public static PVector floorToScreen(PVector floor, PVector wsize) {
		float scaling=Math.min(wsize.x/(rawmaxx-rawminx),wsize.y/(rawmaxy-rawminy));
		PVector mid=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2,(Tracker.rawminy+Tracker.rawmaxy)/2);
		PVector result=new PVector(-(floor.x-mid.x)*scaling+wsize.x/2,-(floor.y-mid.y)*scaling+wsize.y/2);
		return result;
	}
	
	public static PVector screenToFloor(PVector screen, PVector wsize) {
		float scaling=Math.min(wsize.x/(rawmaxx-rawminx),wsize.y/(rawmaxy-rawminy));
		PVector mid=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2,(Tracker.rawminy+Tracker.rawmaxy)/2);
		PVector result=new PVector((wsize.x/2-screen.x)/scaling+mid.x,(wsize.y/2-screen.y)/scaling+mid.y);
		return result;
	}
	*/
	
	public static PVector normalizedToFloor(PVector mapped) {
		PVector result=new PVector(mapped.x,mapped.y);
		result.x=mapped.x*(Tracker.maxx-Tracker.minx)/2.0f;
		result.y=mapped.y*(Tracker.maxy-Tracker.miny)/2.0f;
	//	result.y=-result.y;
		result.x=-result.x;
		result.rotate((float)Math.toRadians(Tracker.screenrotation));
		PVector mid=new PVector((Tracker.rawminx+Tracker.rawmaxx)/2,(Tracker.rawminy+Tracker.rawmaxy)/2);
		result=PVector.add(result,mid);
		return result;
	}
	
	public void resetcoords() {
		PVector tl=floorToNormalized(Tracker.rawminx,Tracker.rawmaxy);
		PVector tr=floorToNormalized(Tracker.rawmaxx,Tracker.rawmaxy);
		PVector bl=floorToNormalized(Tracker.rawminx,Tracker.rawminy);
		PVector br=floorToNormalized(Tracker.rawmaxx,Tracker.rawminy);
		Tracker.minx=Math.min(Math.min(tl.x,tr.x),Math.min(bl.x,br.x));
		Tracker.maxx=Math.max(Math.max(tl.x,tr.x),Math.max(bl.x,br.x));
		Tracker.miny=Math.min(Math.min(tl.y,tr.y),Math.min(bl.y,br.y));
		Tracker.maxy=Math.max(Math.max(tl.y,tr.y),Math.max(bl.y,br.y));
		//PApplet.println("Min/max raw:  "+Tracker.rawminx+":"+Tracker.rawmaxx+", "+Tracker.rawminy+":"+Tracker.rawmaxy);
		//PApplet.println("Min/max scrn: "+Tracker.minx+":"+Tracker.maxx+", "+Tracker.miny+":"+Tracker.maxy);
	}

	synchronized public void pfstarted() {
		PApplet.println("PF started");
	}

	synchronized public void pfstopped() {
		PApplet.println("PF stopped");
		people.clear();
	}

	void pfframe(int frame) {
		//PApplet.println("Got frame "+frame);
		if (frame%50 ==0) {
			OscMessage msg = new OscMessage("/health/VD");
			msg.add((frame%100==0)?1.0:0.0);
			sendOSC("TO",msg);
		}
		lastFrameReceived=frame;
	}

	synchronized void add(int id, int channel) {
		people.add(id, channel);
	}

	// Send fake /pf/* messages for mouse movement
	void sendMouseOSC() {
		int frame=lastFrameReceived;
		float elapsed=0.0f;
		for (int id: mousePeople.pmap.keySet()) {
			Person p=mousePeople.get(id);
//			PApplet.println("Sending data for mouse person "+p.id);
			// Do some rudimentary grouping
			p.groupid=p.id;
			p.groupsize=1;
			for (int id2: mousePeople.pmap.keySet()) {
				if (id2!=id) {
					Person p2=mousePeople.get(id2);
					float dist=PVector.dist(p.getOriginInMeters(),p2.getOriginInMeters());
					if (dist<0.5f) {
						p.groupid=Math.min(id,id2);
						p.groupsize++;
//						PApplet.println("Mouse "+id+" and "+id2+" grouped with distance "+dist);
					}
				}
			}
			OscMessage msg;
			if (genFrameMsgs) {
				msg=new OscMessage("/pf/frame");
				msg.add(frame);
				Laser.getInstance().sendMessage(msg); sendOSC("VD",msg);
			}
			msg=new OscMessage("/pf/update");
			msg.add(frame);
			msg.add(elapsed); // Elapsed time
			msg.add(p.id);
			msg.add(p.getOriginInMeters().x);
			msg.add(p.getOriginInMeters().y);
			msg.add(p.getVelocityInMeters().x);
			msg.add(p.getVelocityInMeters().y);
			msg.add(0.0f);  // Major axis
			msg.add(0.0f);  // Minor axis
			msg.add(p.groupid); // Groupd ID
			msg.add(p.groupsize);   // Group size
			msg.add(p.channel);   // channel
			sendOSC("Laser",msg); sendOSC("VD",msg);

			msg=new OscMessage("/pf/body");
			msg.add(frame);
			msg.add(p.id);
			msg.add(p.getOriginInMeters().x);
			msg.add(p.getOriginInMeters().y);
			msg.add(0.0f);  // ex
			msg.add(0.0f);  // ey
			msg.add(p.getVelocityInMeters().mag());  // speed
			msg.add(0.0f);  // espeed
			msg.add((float)(p.getVelocityInMeters().heading()*180/Math.PI-90));  // heading		
			msg.add(0.0f);  // eheading
			msg.add(0.0f);  // Facing
			msg.add(0.0f);  // Efacing
			msg.add(p.diam); // Diameter
			msg.add(0.0f);   // Sigma(diameter)
			msg.add(p.sep);   // Leg sep
			msg.add(0.0f);    // Leg sep sigma
			msg.add(0.0f);    // Leftness
			msg.add(1);  	// Visibility
			sendOSC("Laser",msg); sendOSC("VD",msg);

			for (int i=0;i<p.legs.length;i++) {
				Leg leg=p.legs[i];
				msg=new OscMessage("/pf/leg");
				msg.add(frame);
				msg.add(p.id);
				msg.add(i);
				msg.add(p.legs.length);
				msg.add(leg.getOriginInMeters().x);
				msg.add(leg.getOriginInMeters().y);
				msg.add(0.0f);  // error in x
				msg.add(0.0f);  // error in y
				msg.add(leg.getVelocityInMeters().mag()); // speed
				msg.add(0.0f);  // espd
				msg.add((float)(leg.getVelocityInMeters().heading()*180/Math.PI-90)); // heading
				msg.add(0.0f);   // eheading
				msg.add(1);      // visibility
				sendOSC("Laser",msg); sendOSC("VD",msg);
			}
		}
	}

	void clearMice() {
		OscMessage msg=new OscMessage("/pf/set/npeople");
		msg.add(0);
		sendOSC("Laser",msg); sendOSC("VD",msg);
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

		if (xpos<Tracker.rawminx-1) {
			PApplet.println("Got xpos ("+xpos+") less than minx ("+Tracker.rawminx+")");
		}
		if (xpos>Tracker.rawmaxx+1) {
			PApplet.println("Got xpos ("+xpos+") greater than maxx ("+Tracker.rawmaxx+")");
		}
		if (ypos<Tracker.rawminy-1) {
			PApplet.println("Got ypos ("+ypos+") less than miny ("+Tracker.rawminy+")");
		}
		if (ypos>Tracker.rawmaxy+1) {
			PApplet.println("Got ypos ("+ypos+") greater than maxy ("+Tracker.rawmaxy+"),");
		}

		people.getOrCreate(id,channel).move(new PVector(xpos, ypos), new PVector(xvelocity, yvelocity), groupid, groupsize, elapsed);
	}
	
	synchronized public void pfbody(int sampnum,int id,
			float x,float y,float ex,float ey,
			float spd,float espd,float heading,float eheading,
			float facing,float efacing,
			float diam,float sigmadiam,
			float sep,float sigmasep,
			float leftness,int visibility) {
		Person p=people.get(id);
		if (p!=null) {
			p.setLegSeparation(sep);
			p.setLegDiameter(diam);
		}
	}
	synchronized public void pfleg(int sampnum,int id,int leg,int nlegs,
			float x,float y,float ex,float ey,
			float spd,float espd,float heading,float eheading,
			int visibility) {
		Person p=people.get(id);
		if (p!=null)
			p.legs[leg].move(new PVector(x,y),new PVector((float)(-spd*Math.sin(heading*Math.PI/180)),(float)(spd*Math.cos(heading*Math.PI/180))));
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

	public void pfbackground(int scan, int nscan, float angle, float range) {
		// Not implemented.
	}
	
	public void pfgeo(int frame, int id, float centerDist, float otherDist, float existDist) {
		// Not implemented.
	}
	
	public void pfgroup(int frame, int gid, int gsize, float life, float centroidX, float centroidY, float diameter) {
		// Not implemented.
	}
	
	public void cycle() {
		int newvis=currentvis;
		do {
			newvis=(newvis+1)%vis.length;
		} while (!selectable[newvis]);
		setapp(newvis);
//		Calendar cal=Calendar.getInstance();
//		int hour=cal.get(Calendar.HOUR_OF_DAY);
//		PApplet.println("Autocycling hour = "+hour);
//		cycler.change(hour>=7 && hour <= 19);
	}
	
	synchronized public void pfsetnpeople(int n) {
		PApplet.println("/pf/set/npeople: now have "+n+" people, size="+people.pmap.size());
		if (n==0)
			setapp(currentvis);   // Cause a reset
		if (n==0 && people.pmap.size()>0 && autocycle)
			cycle();
		people.setnpeople(n);  // Also clears positions
	}

	synchronized public void pfexit(int sampnum, float elapsed, int id) {
		PApplet.println("exit: sampnum="+sampnum+", elapsed="+elapsed+", id="+id);
		people.exit(id);
		if (people.pmap.size()==0 && autocycle)
			cycle();
	}

	synchronized public void pfentry(int sampnum, float elapsed, int id, int channel) {
		add(id,channel);
		PApplet.println("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel+", color="+people.get(id).getcolor());
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

