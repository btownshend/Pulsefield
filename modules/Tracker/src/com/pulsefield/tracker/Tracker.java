package com.pulsefield.tracker;
import java.awt.Color;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

import codeanticode.syphon.SyphonServer;
import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import oscP5.OscProperties;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PMatrix2D;
import processing.core.PMatrix3D;
import processing.core.PVector;
import processing.opengl.PGraphicsOpenGL;
import processing.opengl.PJOGL;

public class Tracker extends PApplet {
	/**
	 * 
	 */
	public static final String SVGDIRECTORY="SVGFiles/";
	private static boolean present = false;
	private static boolean autocycle = false;
	private static boolean starting = true;   // Disable bad OSC messages before setup
	private static boolean genFrameMsgs = false;
	private static int numProjectors = 4;
	@SuppressWarnings("unused")
	private static final long serialVersionUID = 1L;
	private int tick=0;
	private int liveTick=0;  // Tick at which last AL message received
	public float avgFrameRate=0;
	float targetFramerate = 30;
	public static OscP5 oscP5;
	NetAddress myRemoteLocation;
	static final float screenrotation=0f; // 90f;   // Rotate raw coordinates CCW by this number of degrees
	static float lidarminy=-5f, lidarmaxy=5f, lidarminx=-5f, lidarmaxx=5f, lidarRotation=0f; // Bounds of tracking
	static float miny, maxy, minx, maxx; // Bound of video display (in LIDAR coordinates - meters)
	static Visualizer vis[] = new Visualizer[0];
	VisualizerGrid visGrid;
	VisualizerMeasure visMeasure;
	VisualizerNavier visNavier;
	VisualizerNavierOF visNavierOF;
	VisualizerDDR visDDR;
	VisualizerMenu visMenu;
	VisualizerMinim visMinim;
	VisualizerSyphon visSyphon;
	VisualizerRainbow visRainbow;
	VisualizerWords visWords;
	VisualizerGravity visGravity;
	VisualizerGuitar visGuitar;
	VisualizerVoronoi visVoronoi;
	VisualizerProximity visProximity;
	VisualizerPads visPads;
	
	int currentvis=-1;
	static NetAddress TO, OF, AL, MAX, CK, VD, FE, AR;
	People people, mousePeople;
	Ableton ableton;
	boolean useMAX;
	Synth synth;
	TouchOSC touchOSC;
	OFOSC oFOSC;
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
	final static int CANVASAREA=1200*600;   // Area of canvas -- aspect depends on active area
	PGraphicsOpenGL canvas;
	Projector projectors[];
	Config jconfig;
	static PVector alignCorners[][]=new PVector[2][0];
	static ProjCursor cursors[]=null;
	Map<String,Boolean> unhandled;
	PVector[][] lidar = new PVector[2][571];
	PGraphicsOpenGL mask[];
	int pselect[];
	boolean drawBounds=false;   // True to overlay projector bounds
	boolean drawBorders=false;   // True to overlay active area bounds
	boolean drawMasks = false;
	boolean useMasks = true;
	boolean showProjectors = false;
	boolean enableMenu = true;   // True to enable menu
	boolean enableTitle = true;
	static Tracker theTracker;
    static String pfroot;
    private final static Logger logger = Logger.getLogger(Tracker.class.getName());

    
	public void settings() {
		// If Tracker uses FX2D or P2D for renderer, then we can't do 3D and vortexRenderer will be blank!
		size(1280, 720, renderer);
		PJOGL.profile=1;    // Seems that this is needed for Syphon to function
		//pixelDensity(2);  // This breaks the Navier visualizer
	}

	public void setup() {
		logger.entering(this.getClass().getName(), "setup");
		theTracker = this;   // Keep a static copy of the (sole) tracker instance

		pfroot = System.getenv("PFROOT");
		if (pfroot==null) {
			logger.severe("PFROOT environment variable should be set to root of Pulsefield tree");
			System.exit(1);
		}
		logger.config("PFROOT="+pfroot);
		configFile=pfroot+"/config/urlconfig.txt";   // FIXME: Make this relative to launch directory
		jconfig=new Config(pfroot+"/config/tracker-config.json");
		Config.load(this);
		minx=Config.getFloat("video", "minx", -5f);
		maxx=Config.getFloat("video", "maxx", 5f);
		miny=Config.getFloat("video", "miny", 0f);
		maxy=Config.getFloat("video", "maxy", 5f);
		
		try {
			config=new URLConfig(configFile);
		} catch (IOException e) {
			logger.severe("Failed to open URL config: "+configFile+": "+e.getLocalizedMessage());
			logger.severe("PFROOT currently set to "+pfroot+".  Is that the root of the Pulsefield repository installation?");
			System.exit(1);
		}

		frameRate(targetFramerate);
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
		OF = new NetAddress(config.getHost("OF"), config.getPort("OF"));
		AL = new NetAddress(config.getHost("AL"), config.getPort("AL"));
		CK = new NetAddress(config.getHost("CK"), config.getPort("CK"));
		VD = new NetAddress(config.getHost("VD"), config.getPort("VD"));
		FE = new NetAddress(config.getHost("FE"), config.getPort("FE"));
		AR = new NetAddress(config.getHost("AR"), config.getPort("AR"));
		logger.config("Sending to myself at "+VD.address()+":"+VD.port());
		logger.config("Sending chuck commands to "+config.getHost("CK")+":"+config.getPort("CK"));
		logger.config("AL at "+config.getHost("AL")+":"+config.getPort("AL"));
		MAX = new NetAddress(config.getHost("MAX"), config.getPort("MAX"));
		touchOSC = new TouchOSC(oscP5, TO);
		oFOSC = new OFOSC(oscP5,OF);
		ableton = new Ableton(oscP5, AL);
		LEDs.theLEDs=new LEDs(AR.address(),AR.port());
		
		new Laser(oscP5, new NetAddress(config.getHost("LASER"), config.getPort("LASER")));
		synth = new Max(this,oscP5, MAX);

		synth.play(0,64,100,100,1);
		Effects.setupDefault(synth);
		
		prevMousePos=new PVector(0f,0f);
		prevMousePressed=false;
		mouseVel=new PVector(0f,0f);
		
		// Facility for real-time FFT of default input
		fourier=new Fourier(this);
		
		// Setup OSC handlers
		oscP5.plug(this, "pfframe", "/pf/frame");
		oscP5.plug(this, "pfupdate", "/pf/update");
		oscP5.plug(this, "pfscanpt","/pf/scanpt");
		oscP5.plug(this, "pfoutsiders","/pf/outsiders");
		oscP5.plug(this, "pfaligncorner","/pf/aligncorner");
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
		oscP5.plug(this, "pfsetrotation", "/pf/set/rotation");
		oscP5.plug(this, "pfstarted", "/pf/started");
		oscP5.plug(this, "pfstopped", "/pf/stopped");	
		oscP5.plug(this, "tempo", "/tempo");
		oscP5.plug(this, "volume", "/volume");
		oscP5.plug(this, "ping", "/ping");
		oscP5.plug(this,  "setcursor", "/cal/cursor");
		oscP5.plug(this, "setscreen2world","/cal/screen2world");
		oscP5.plug(this, "setworld2screen","/cal/world2screen");
		oscP5.plug(this, "setpose","/cal/pose");
		oscP5.plug(this, "setcameraview","/cal/cameraview");
		oscP5.plug(this, "setprojection","/cal/projection");
		oscP5.plug(this, "setborders", "/video/borders");
		oscP5.plug(this, "setbounds", "/video/bounds");
		oscP5.plug(this, "menuenable", "/video/menu");
		oscP5.plug(this, "setminx", "/video/minx");
		oscP5.plug(this, "setmaxx", "/video/maxx");
		oscP5.plug(this, "setminy", "/video/miny");
		oscP5.plug(this, "setmaxy", "/video/maxy");
		oscP5.plug(this,"locklidar","/video/locklidar");
		unhandled = new HashMap<String,Boolean>();
		// Add some known unhandled messages to avoid warnings
		unhandled.put("/ping",true);	
		projectors=new Projector[numProjectors];
		for (int i=0;i<numProjectors;i++)
			projectors[i] = new Projector(this,i+1,1920,1080);

		mask=new PGraphicsOpenGL[projectors.length];
		resetcoords();

		logger.fine("settings() complete");
	}
	
	private void makeCanvases() {
		float area=(maxx-minx)*(maxy-miny);
		float sc=sqrt(CANVASAREA/area);
		int CANVASWIDTH=(int)(sc*(maxx-minx));
		int CANVASHEIGHT=(int)(sc*(maxy-miny));
		if (canvas==null || canvas.width!=CANVASWIDTH || canvas.height!=CANVASHEIGHT) {
			logger.config("Making new canvas: "+CANVASWIDTH+"x"+CANVASHEIGHT);
			canvas = (PGraphicsOpenGL) createGraphics(CANVASWIDTH, CANVASHEIGHT, renderer);
			float mscale=8;	// How much smaller to make the masks
			for (int i=0;i<mask.length;i++) {
				mask[i]=(PGraphicsOpenGL) createGraphics((int)(CANVASWIDTH/mscale+0.5), (int)(CANVASHEIGHT/mscale+0.5),renderer);
				// Set for unmasked in case it is not used
				//			mask[i].noSmooth();
			}
			pselect=new int[mask[0].width*mask[0].height];
			for (int i=0;i<pselect.length;i++)
				pselect[0]=0;  // Default to projector 0
		}
	}

	private void addVisualizers() {
		Scale scale=new Scale("Major","C");
		addVis("Pads", visPads = new VisualizerPads(this, synth),false);
		addVis("Navier",visNavier=new VisualizerNavier(this,synth),true); 
		addVis("NavierOF",visNavierOF=new VisualizerNavierOF(this,synth,"GPUFluidsDebug"),true); 
		addVis("Tron",new VisualizerTron(this,scale,synth),true);
		addVis("Grid",visGrid=new VisualizerGrid(this),true);
		oscP5.plug(visGrid,  "songIncr", "/touchosc/song/incr");
		addVis("DDR",visDDR=new VisualizerDDR(this),true);
		addVis("Poly",new VisualizerPoly(this,scale,synth),true);
		addVis("Voronoi",visVoronoi = new VisualizerVoronoi(this,scale,synth),true);
		addVis("Guitar", visGuitar = new VisualizerGuitar(this,synth),true);
		addVis("Dot",new VisualizerDot(this),false);
		addVis("CHucK",new VisualizerChuck(this),false);
		addVis("Proximity",visProximity = new VisualizerProximity(this),true);
		addVis("Cows",new VisualizerCows(this),true);
		addVis("Trump",new VisualizerWhack(this,"whack","Whack","WHACK"),false);
		addVis("Bowie",new VisualizerZiggy(this,"bowie","Bowie","WHACK-Bowie"),true);
		addVis("Soccer",new VisualizerSoccer(this),true);
		addVis("Menu",visMenu=new VisualizerMenu(this),false);
		addVis("Visualizer",visMinim=new VisualizerMinim(this,fourier,renderer==P3D),false);
		addVis("Calypso",new VisualizerCalypso(this,synth),true);
		addVis("DNA",new VisualizerDNA(this),true);
		addVis("Syphon",visSyphon=new VisualizerSyphon(this,"TCPSyphonClient","Default(31053)"),false);
		//visSyphon = new VisualizerSyphon(this,"Syphoner","Evernote");
		//visSyphon = new VisualizerSyphon(this,"Tutorial","Main Camera");
		//addVis("Balls",new VisualizerUnity(this,"Tutorial","Balls.app"),true);
		addVis("Osmos",new VisualizerOsmos(this,synth),true);
		addVis("Freeze",new VisualizerFreeze(this,synth),true);
		addVis("VDMX",new VisualizerVDMX(this,Tracker.pfroot+"/../VDMX/Projects/ValentinesDayStarter/Valentines.vdmx5"),false);
		addVis("Measure",visMeasure=new VisualizerMeasure(this),true);
		addVis("Life", new VisualizerLife(this),true);
		addVis("Stickman",new VisualizerStickman(this,synth),true);
		addVis("Hunter", new VisualizerHunter(this,synth),true);
		addVis("Gravity", visGravity = new VisualizerGravity(this), true);
		addVis("Rainbow", visRainbow = new VisualizerRainbow(this), true);
		addVis("Words", visWords = new VisualizerWords(this), true);
		addVis("TestPattern",new VisualizerTestPattern(this),false);
		setapp(vis.length-1);
	}
	
	public void addVis(String name, Visualizer v,boolean selectable) {
		// Add a visualizer to the internal list
		Visualizer tvis[]=vis;
		vis=new Visualizer[tvis.length+1];
		for (int i=0;i<tvis.length;i++)
			vis[i]=tvis[i];
		v.setName(name);
		v.setSelectable(selectable);
		// Set pos, selectable
		vis[tvis.length]=v;
		logger.fine("Created "+name);
	}
	
	public void tempo(float t) {
		logger.fine("tempo("+t+")");
		MasterClock.settempo(t);
		Ableton.getInstance().setALTempo(t);
	}

	public void volume(float vol) {
		Ableton.getInstance().setALVolume(vol);
	}
	
	public void ping(int code) {
		OscMessage msg = new OscMessage("/ack");
		//logger.fine("Got ping "+code);
		msg.add(code);
		//		oscP5.send(msg,MPO);
	}

	public void setborders(float onoff) {
		OscMessage msg = new OscMessage("/video/borders");
		logger.info("Got /video/borders,"+onoff);
		msg.add(onoff);
		oscP5.send(msg,TO);
		drawBorders=onoff!=0f;
		GUI.theGUI.update();
	}
	
	public void setbounds(float onoff) {
		OscMessage msg = new OscMessage("/video/bounds");
		logger.info("Got /video/bounds,"+onoff);
		msg.add(onoff);
		oscP5.send(msg,TO);
		drawBounds=onoff!=0f;
		GUI.theGUI.update();
	}
	
	public void menuenable(float onoff) {
		OscMessage msg = new OscMessage("/video/menu");
		logger.info("Got /video/menu,"+onoff);
		msg.add(onoff);
		oscP5.send(msg,TO);
		enableMenu=onoff!=0f;
		GUI.theGUI.update();
	}
	
	synchronized public void setminx(float minx) {
		if (minx<maxx-1) {
			Tracker.minx=minx;
			resetcoords();
		}
	}
	synchronized public void setmaxx(float maxx) {
		if (maxx>minx+1) {
			Tracker.maxx=maxx;
			resetcoords();
		}
	}
	synchronized public void setminy(float miny) {
		if (miny<maxy-1) {
			Tracker.miny=miny;
			resetcoords();
		}
	}
	synchronized public void setmaxy(float maxy) {
		if (maxy>miny+1) {
			Tracker.maxy=maxy;
			resetcoords();
		}
	}
	
	public void setcursor(int cursor, int ncursor, int proj, float x, float y) {
		logger.fine("setcursor("+proj+","+cursor+","+ncursor+","+x+","+y+")");
		if (ncursor==0)
			cursors=null;
		else if (cursors==null || cursors.length != ncursor) {
			logger.fine("setcursor: resizing to "+ncursor);
			cursors=new ProjCursor[ncursor];
		}
		if (ncursor>0) {
			assert(cursor<ncursor);
			cursors[cursor]=new ProjCursor(proj, x, y);
		}
	}
	
	public void setscreen2world(int proj, float x00, float x01, float x02, float x10, float x11, float x12, float x20, float x21, float x22) {
		logger.entering("Tracker","setscreen2world",""+proj+","+x00+","+x01+"...)");
		if (proj<0 || proj>=projectors.length) {
			logger.warning("setscreen2world: Bad projector number: "+proj);
			return;
		}
		PMatrix3D mat=new PMatrix3D();
		mat.set(x00,x01,0,x02,
				x10,x11,0,x12,
				  0,  0,1,  0f,
				x20,x21,0,x22);
		projectors[proj].setScreen2World(mat);
	}
	
	public void setworld2screen(int proj, float x00, float x01, float x02, float x10, float x11, float x12, float x20, float x21, float x22) {
		logger.finer("setworld2screen("+proj+","+x00+","+x01+"...)");
		if (proj<0 || proj>=projectors.length) {
			logger.warning("setworld2screen: Bad projector number: "+proj);
			return;
		}
		PMatrix3D mat=new PMatrix3D();
		mat.set(x00,x01,0,x02,
				x10,x11,0,x12,
				  0,  0,0,  0.5f,
				x20,x21,0,x22);
		projectors[proj].setWorld2Screen(mat);
	}
	
	public void setprojection(int proj, float x00, float x01, float x02, float x10, float x11, float x12) {
		PMatrix2D mat=new PMatrix2D();
		mat.set(x00,x01,x02,
				x10,x11,x12);
		logger.finer("setprojection("+proj+":");
		mat.print();
		projectors[proj].setProjection(mat);
	}
	
	public void setpose(int proj, float x, float y, float z) {
		PVector vec=new PVector(x,y,z);
		logger.finer("setpose("+proj+","+vec+")");
		projectors[proj].setPosition(vec);
	}

	public void setcameraview(int proj, float x00, float x01, float x02, float x03, float x10, float x11, float x12, float x13, float x20, float x21, float x22, float x23) {
		PMatrix3D mat=new PMatrix3D();
		mat.set(x00,x01,x02,x03,
				x10,x11,x12,x13,
				x20,x21,x22,x23,
				0,0,0,1);
		logger.finer("setcameraview("+proj+":");
		mat.print();
		projectors[proj].setCameraView(mat);
	}

	private String vispos(int i) {
		final int numrows=7;
		int row=i/5;
		int col=i-row*5;
		return ""+(numrows-row)+"/"+(col+1);
	}
	
	public void vsetapp(OscMessage msg) {
		for (int i=0;i<vis.length;i++) {
			if (msg.checkAddrPattern("/video/app/buttons/"+vispos(i)) ) {
				setapp(i);
				return;
			}		
		}
		logger.warning("Bad vsetup message: "+msg);
	}

	public static void sendOSC(String dest, OscMessage msg) {
		if (dest.equals("AL"))
			oscP5.send(msg,AL);
		else if (dest.equals("TO"))
			oscP5.send(msg,TO);
		else if (dest.equals("CK"))
			oscP5.send(msg,CK);
		else if (dest.equals("Laser"))
			Laser.getInstance().sendMessage(msg);
		else if (dest.equals("VD"))
			oscP5.send(msg,VD);
		else if (dest.equals("FE"))
			oscP5.send(msg,FE);
		else
			logger.severe("sendOSC: Bad destination: "+dest);
	}

	public static void sendOSC(String dest, String path, int data) {
		OscMessage msg=new OscMessage(path);
		msg.add(data);
		sendOSC(dest,msg);
	}

	public static void sendOSC(String dest, String path, float data) {
		OscMessage msg=new OscMessage(path);
		msg.add(data);
		sendOSC(dest,msg);
	}
	
	public static void sendOSC(String dest, String path, String data) {
		OscMessage msg=new OscMessage(path);
		msg.add(data);
		sendOSC(dest,msg);
	}

	public int getAppIndex(String name) {
		// Return index of app called 'name' or -1 if none
		for (int i=0;i<vis.length;i++)
			if (vis[i].name==name)
				return i;
		return -1;
	}
	
	synchronized public void setapp(int appNum) {
		if (appNum <0 || appNum > vis.length) {
			println("Bad video app number: "+appNum);
			return;
		}
		if (appNum==currentvis)
			return;
		// Turn off old block
		for (int k=0; k<vis.length;k++)
			if (k!=appNum) {
				OscMessage msg = new OscMessage("/video/app/buttons/"+vispos(k));
				msg.add(0);
				sendOSC("TO",msg);
				//logger.fine("Sent "+msg.toString());
			}

		if (currentvis!=-1)
			vis[currentvis].stop();
		currentvis=appNum;
		logger.info("Switching to app "+currentvis+": "+vis[currentvis].name);
		vis[currentvis].setName(vis[currentvis].name);
		// Turn on block for current app
		OscMessage msg = new OscMessage("/video/app/buttons/"+vispos(currentvis));
		msg.add(1.0);
		sendOSC("TO",msg);

		msg = new OscMessage("/video/app/name");
		msg.add(vis[currentvis].name);
		sendOSC("TO",msg);
		vis[currentvis].start();
		if (GUI.theGUI != null)
			GUI.theGUI.update();
	}

	synchronized public void draw() {
		try {
		if (starting) {
			// Setup visualizers at first draw
			addVisualizers();
			GUI.start();
			logger.config("Finished initialization");
			starting = false;
		}
		tick++;
		avgFrameRate=avgFrameRate*(1f-1f/20f)+frameRate/20f;
		if (GUI.theGUI != null)
			GUI.theGUI.updateDisplayValues();
		if (tick%20 == 0)
			updateTO(tick%40==0);
		
		canvas.beginDraw();
		// Transform so that coords for drawing are in meters
		canvas.resetMatrix();

		canvas.translate(canvas.width/2, canvas.height/2);   // center coordinate system to middle of canvas
		// The LIDAR uses a RH x-y axis (with origin top-center), but PGraphics uses a LH (with origin top-left)
		// So, we can flip the x-axis to get things aligned
		canvas.scale(getPixelsPerMeter(),getPixelsPerMeter()); 
		canvas.translate(-getFloorCenter().x, -getFloorCenter().y);  // translate to center of new space

		float cscale=Math.min((width-2)*1f/canvas.width,(height-2)*1f/canvas.height); // Scaling of canvas to window
		//println("cscale="+cscale);
		if (mousePressed) {
			Person p=mousePeople.getOrCreate(mouseID,mouseID%16);
			//PVector sMousePos=normalizedToFloor(new PVector(mouseX*2f/width-1, mouseY*2f/height-1));
			float cmouseX=-(mouseX-width/2)/cscale/getPixelsPerMeter()+(minx+maxx)/2;
			float cmouseY=(mouseY-height/2)/cscale/getPixelsPerMeter()+(miny+maxy)/2;
			PVector mousePos=new PVector(cmouseX, cmouseY);
			//println("mouse="+mouseX+", "+mouseY+" -> "+cmouseX+", "+cmouseY);
			
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
//			logger.fine("Moved mouse ID "+mouseID+" to "+mousePos+" with velocity "+p.getVelocityInMeters());
		} else {
			mouseVel.set(0f,0f);
		}
		prevMousePressed=mousePressed;
		if (enableMenu && visMenu.hotSpotCheck(this,people))
			setapp(getAppIndex("Menu"));

		sendMouseOSC();

		vis[currentvis].update(this, people);
		//		translate((width-height)/2f,0);

		canvas.pushStyle();
		canvas.pushMatrix();
		vis[currentvis].draw(this, canvas,people);
		canvas.popMatrix();
		canvas.popStyle();
		if (enableTitle) {
			canvas.pushStyle();
			canvas.textAlign(PConstants.CENTER, PConstants.CENTER);
			Visualizer.drawText(canvas, 0.1f, "The Pulsefield", minx+0.3f*(maxx-minx), miny+0.9f*(maxy-miny));
			canvas.popStyle();
		}
		if (enableMenu && vis[currentvis]!=visMenu) {
			canvas.pushStyle();
			visMenu.hotSpotDraw(canvas);
			canvas.popStyle();
		}
		
		//vis[currentvis].drawLaser(this,people);


//		if (drawBounds) {
//			canvas.pushStyle();
//			canvas.strokeWeight(0.025f);
//			canvas.noFill();
//			for (int i=0;i<projectors.length;i++) {
//				canvas.stroke(i==0?255:0,i==1?255:0,i>1?255:0);
//				canvas.beginShape();
//				for (int j=0;j<projectors[i].bounds.length;j++) {
//					PVector p=projectors[i].bounds[j];
//					canvas.vertex(p.x,p.y);
//					//logger.fine("vertex("+projectors[i].bounds[j]+") -> "+canvas.screenX(p.x,p.y)+","+canvas.screenY(p.x, p.y));
//				}
//				canvas.endShape(CLOSE);
//			}
//			canvas.popStyle();
//		}
		
		canvas.endDraw();
		// Syphon setup, requires OpenGL renderer (not FX2D?)
		if (useSyphon && server==null) {
			logger.config("Creating new syphon server...");
			try {
				server = new SyphonServer(this, "Tracker");
			} catch (Exception e) {
			    logger.log(Level.SEVERE,"Exception while creating syphon server",e);
				e.printStackTrace();
				exit();
			}
			logger.config("Done creating new syphon server");
		}
		
		if (server != null) {
			server.sendImage(canvas);
		}
		if (useMasks)
			buildMasks(canvas);
		for (int i=0;i<projectors.length;i++)
			projectors[i].render(canvas,useMasks?mask[i]:null,drawBounds);

		imageMode(CENTER);
		// Canvas is RH, so need to flip it back to draw on main window (which is LH)
		pushMatrix();
		translate(width/2,height/2);
		scale(-1,1);
		translate(-width/2,-height/2);
		background(0);
		image(canvas, width/2, height/2, canvas.width*cscale, canvas.height*cscale);		
		popMatrix();

		if (showProjectors || drawMasks) {
			// Use top-left, top-right corners for projector images
			float pfrac = 0.25f;  // Use this much of the height of the window for projs
			float pheight=this.height*pfrac;
			stroke(255,0,0);
			strokeWeight(2);
			imageMode(CORNER);
			assert(projectors.length<=4);
			for (int i=0;i<projectors.length;i++) {
				float pwidth=pheight*projectors[i].pcanvas.width/projectors[i].pcanvas.height; 
				float x=(i%2==0)?0:width-pwidth;
				float y=(i<2)?0:height-pheight;
				rect(x, y, pwidth, pheight);
				if (drawMasks)
					image(mask[i], x+1, y+1f, pwidth-2, pheight-2);
				else
					image(projectors[i].pcanvas, x+1, y+1f, pwidth-2, pheight-2);
			}
		}
		//SyphonTest.draw(this);
		Config.saveIfModified(this);   // Save if modified
		} catch (Exception e) {
		    logger.log(Level.SEVERE,"exception in draw(): ",e);
		    e.printStackTrace();
		}
	}

	private void buildMasks(PGraphicsOpenGL canvas) {
		for (int i=0;i<mask.length;i++) {
			mask[i].beginDraw();
			// Transform so that coords for drawing are in meters
			mask[i].resetMatrix();
			mask[i].translate(mask[i].width/2, mask[i].height/2);   // center coordinate system to middle of canvas
			mask[i].scale(getPixelsPerMeter()*mask[i].width/canvas.width, getPixelsPerMeter()*mask[i].height/canvas.height);
			mask[i].translate(-getFloorCenter().x, -getFloorCenter().y);  // translate to center of new space

			mask[i].blendMode(PConstants.REPLACE);
			mask[i].imageMode(PConstants.CORNER);
			mask[i].ellipseMode(PConstants.CENTER);
			mask[i].background(0);  // default, nothing drawn
			mask[i].fill(255);      // Can draw anywhere within bounds
			// use a stroke to create an inset to prefer the other projector near the edges
			//mask[i].stroke(200);
			//mask[i].strokeWeight(0.2f);
			mask[i].noStroke();
			mask[i].beginShape();
			for (int j=0;j<projectors[i].bounds.length;j++) {
				PVector p=projectors[i].bounds[j];
				mask[i].vertex(p.x,p.y);
				//logger.fine("vertex("+projectors[i].bounds[j]+") -> "+canvas.screenX(p.x,p.y)+","+canvas.screenY(p.x, p.y));
			}
			mask[i].endShape(CLOSE);

			PVector projPos=projectors[i].pos;projPos.z=0;
			mask[i].blendMode(PConstants.MULTIPLY);  // Only affect the non-zero pixels
			mask[i].fill(127);  // Don't draw here, but not as insistent as being out of bounds
			float fardist=10f;  // Far distance of shadow
			float shadowOffset=0.1f;  // Distance beyond leg centers to begin shadow
			for (Person ps: people.pmap.values()) {  
				PVector l1=ps.legs[0].getOriginInMeters();
				PVector l2=ps.legs[1].getOriginInMeters();
				PVector pos=PVector.mult(PVector.add(l1, l2), 0.5f);
				PVector toPos=PVector.sub(pos, projPos); toPos.normalize();
				PVector ra=toPos.cross(new PVector(0,0,1)); ra.normalize();
				PVector l1tol2=PVector.sub(l2, l1);
				float nearwidth=l1tol2.mag()+(ps.legs[0].getDiameterInMeters()+ps.legs[1].getDiameterInMeters())/2;
				PVector near1=PVector.add(pos, PVector.mult(ra, nearwidth/2));
				PVector near2=PVector.add(pos, PVector.mult(ra, -nearwidth/2));
				PVector s1dir=PVector.sub(near1, projPos); s1dir.normalize();
				near1=PVector.add(near1, PVector.mult(s1dir, shadowOffset));
				PVector far1=PVector.add(near1, PVector.mult(s1dir, fardist) );
				PVector s2dir=PVector.sub(near2, projPos); s2dir.normalize();
				near2=PVector.add(near2, PVector.mult(s2dir, shadowOffset));
				PVector far2=PVector.add(near2, PVector.mult(s2dir, fardist) );
				mask[i].beginShape();
				mask[i].vertex(near1.x,near1.y);
				mask[i].vertex(near2.x,near2.y);
				mask[i].vertex(far2.x,far2.y);
				mask[i].vertex(far1.x,far1.y);
				mask[i].endShape(CLOSE);
				//println("shadow of "+l1+","+l2+" from proj@ "+projPos+": "+far1+", "+far2);
			}
			mask[i].loadPixels();
		}
		
		
		int maskcnt[]=new int[pselect.length];
		// pselect is a persistent map of which projector gets which pixel
		for (int i=0;i<pselect.length;i++) {
			if (pselect[i]==mask.length) {
				// was a no-projector condition
				pselect[i]=0;   // May be able to make this persistent and never check this pixel again (unless resized)
			}
			if ((mask[pselect[i]].pixels[i]&0xff)<255) {
				for (int j=0;j<mask.length;j++) {
					if ((mask[j].pixels[i]&0xff) > (mask[pselect[i]].pixels[i]&0xff))
						pselect[i]=j;  // Switch to a better projector
				}
				if ((mask[pselect[i]].pixels[i]&0xff)==0) {
					pselect[i]=mask.length;  // No projector 
				}
			}
			maskcnt[pselect[i]]+=1;
		}
			
		//logger.fine("Mask has projector 0: "+maskcnt[0]+", 1: "+maskcnt[1]+", None: "+maskcnt[2]);
		// Now bring those back to the masks
		int fullon=0xffffffff;
		int fulloff=0xff000000;
		for (int j=0;j<mask.length;j++) {
			mask[j].blendMode(ADD);
			for (int i=0;i<pselect.length;i++) {
				mask[j].pixels[i]=(pselect[i]==j)?fullon:fulloff;
			}
			//mask[j].pixels[10]=47;
			mask[j].updatePixels();
			mask[j].filter(BLUR,1f);
//			mask[j].filter(ERODE);
//			mask[j].filter(DILATE);

//			mask[j].loadPixels();
//			int pcnt[]=new int[256];
//			for (int i=0;i<pselect.length;i++) {
//				pcnt[mask[j].pixels[i]&0xff]+=1;
//			}
//			PApplet.print("mask "+j+": ");
//			for (int i=0;i<pcnt.length;i++)
//				if (pcnt[i]>0)
//					PApplet.print(i+": "+pcnt[i]+", ");
//			PApplet.println("");


			mask[j].endDraw();
		}
	}
	
	public void mouseReleased() {
		//pfexit(0, 0, 98);
		//mouseID=(mouseID-90+1)%10+90;
	}
	
	@Override
	public void keyPressed() {
		logger.info("Mouse key: "+key);
		if (key=='C' || key=='c') {
			mouseID=90;
			mousePeople.pmap.clear();
			clearMice();
		} else if (key>='1' && key<='9')
			mouseID=90+key-'1';
		else if (key=='x'||key=='X') {
			// Current ID exits
			logger.info("Mouse ID "+mouseID+" exitting.");
			mousePeople.pmap.remove(mouseID);
			clearMice();
		} else if (key=='a' || key=='A') {
			// Advance to next app
			cycle();
		}
	}

	private static void setupLogging() throws SecurityException, IOException {
		// Setup logging
		LogManager.getLogManager().readConfiguration(new FileInputStream("logging.properties"));
	}
	
	public static void main(String args[]) {
		try {
			setupLogging();
		} catch (SecurityException | IOException e) {
			// TODO Auto-generated catch block
			PApplet.println("Unable to initialize logging");
			e.printStackTrace();
			System.exit(1);
		}
		if (present)
			PApplet.main(new String[] { "--present","com.pulsefield.tracker.Tracker"});
		else
			PApplet.main(new String[] {"--display=1","com.pulsefield.tracker.Tracker" });
	}

	/* incoming osc message are forwarded to the oscEvent method. */
	synchronized public void oscEvent(OscMessage theOscMessage) {
		try {
		//logger.fine("Got message:"+theOscMessage.toString());
		if (starting)
			return;
		if (theOscMessage.isPlugged() == true) 
			; // Handled elsewhere
		else if (theOscMessage.addrPattern().startsWith("/video/app/buttons") == true)
			vsetapp(theOscMessage);
		else if (theOscMessage.addrPattern().startsWith("/grid")) {
			// Grid messages need to be broadcast to all apps that use Grid, since they each keep their own state
			visGrid.handleMessage(theOscMessage);
			visMeasure.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/live") || theOscMessage.addrPattern().startsWith("/remix/error")) {
			liveTick=tick;
			ableton.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/proj")) {
			String pattern=theOscMessage.addrPattern();
			String components[]=pattern.split("/");
			logger.fine("num comp="+components.length+", comp2="+components[2]);
			if (components.length>=3) {
				int pnum=Integer.parseInt(components[2]);
				if (pnum>=1 && pnum<=projectors.length)
					projectors[pnum-1].handleMessage(theOscMessage);
				else
					logger.warning("Bad projector number: "+pnum);
			}
		} else if (theOscMessage.addrPattern().startsWith("/video/navierOF")) {
			visNavierOF.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/navier")) {
			visNavier.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/syphon")) {
			visSyphon.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/ddr")) {
			visDDR.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/particlefield")) {
			visRainbow.handleMessage(theOscMessage);
			visGravity.handleMessage(theOscMessage);
			visGrid.handleMessage(theOscMessage);
			visGuitar.handleMessage(theOscMessage);
			visVoronoi.handleMessage(theOscMessage);
			visProximity.handleMessage(theOscMessage);
			visPads.handleMessage(theOscMessage);
			visWords.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/midi/pgm")) {
			synth.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/led")) {
			LEDs.theLEDs.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/pf/set")) {
			// logger.warning("Unhandled set message: "+theOscMessage.addrPattern());
		} else if (theOscMessage.addrPattern().startsWith("/vis/")) {
			// logger.warning("Unhandled vis message: "+theOscMessage.addrPattern());
		} else if (!unhandled.containsKey(theOscMessage.addrPattern())) {
			logger.warning("Received an unhandled OSC message: "+theOscMessage.addrPattern());
			theOscMessage.print();
			unhandled.put(theOscMessage.addrPattern(),true);		
		}  /* print the address pattern and the typetag of the received OscMessage */
		} catch (Exception e) {
			logger.log(Level.SEVERE,"oscEvent exception: "+e.getMessage()+"("+e.toString()+")",e);
			e.printStackTrace();
		}
	}

	public static PVector mapVelocity(PVector velInMetersPerSecond) {
		if (velInMetersPerSecond == null) {
			logger.severe("velInMetersPerSecond is null");
			return new PVector(0,0);
		}
		PVector sz=getFloorSize();
		return new PVector(-velInMetersPerSecond.x*2f/sz.x,velInMetersPerSecond.y*2f/sz.y);
	}

	public static PVector floorToNormalized(float x, float y) {
		return floorToNormalized(new PVector(x,y),false);
	}

	// Map position in meters to normalized position where (minx,miny) maps to (-1,1) and (max,maxy) maps to (1,-1)
	public static PVector floorToNormalized(PVector raw, boolean preserveAspect) {
		PVector mid=getFloorCenter();
		PVector sz=getFloorSize();
		PVector result=PVector.sub(raw,mid);
		
		if (preserveAspect)
			result=PVector.mult(result,2f/Math.min(sz.x,sz.y));
		else
			result.set(result.x*2.0f/sz.x,result.y*2.0f/sz.y);
	
//		logger.fine("Mapped ("+raw+") to ("+result);
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
		PVector mid=getFloorCenter();
		PVector sz=getFloorSize();
		PVector result=new PVector(mapped.x*sz.x/2+mid.x,mapped.y*sz.y/2+mid.y);
		return result;
	}
	
	// Called after any update to LIDAR active area or projector bounds
	public void resetcoords() {
		// Choose bounds such that there is good projector coverage
		
		// Find bounds of projected area
		PVector avg=new PVector(0,0);
		float pminx=100,pminy=100,pmaxx=-100,pmaxy=-100;  // Extents of coverage of any pixels
		for (int i=0;i<projectors.length;i++) {
			for (int j=0;j<projectors[i].bounds.length;j++) {
				PVector p=new PVector(projectors[i].bounds[j].x,projectors[i].bounds[j].y);
				//p.y=max(0,p.y);  // Keep in front of LIDAR
				pminx=min(pminx,p.x);
				pminy=min(pminy,p.y);
				pmaxx=max(pmaxx,p.x);
				pmaxy=max(pmaxy,p.y);
				avg.x+=p.x;
				avg.y+=p.y;
			}
		}
		// Constrain to be no more than projected area
		minx=max(minx,pminx);
		maxx=min(maxx,pmaxx);
		miny=max(miny,pminy);
		maxy=min(maxy,pmaxy);

		sendOSC("TO","/video/minx",minx);
		sendOSC("TO","/video/minx/label","Min X: "+String.format("%.1f", minx));
		sendOSC("TO","/video/maxx",maxx);
		sendOSC("TO","/video/maxx/label","Max X: "+String.format("%.1f", maxx));
		sendOSC("TO","/video/miny",miny);
		sendOSC("TO","/video/miny/label","Min Y: "+String.format("%.1f", miny));
		sendOSC("TO","/video/maxy",maxy);
		sendOSC("TO","/video/maxy/label","Max Y: "+String.format("%.1f", maxy));
		
		sendOSC("TO","/pf/minx/label","Min X: "+String.format("%.1f", lidarminx));
		sendOSC("TO","/pf/maxx/label","Max X: "+String.format("%.1f", lidarmaxx));
		sendOSC("TO","/pf/miny/label","Min Y: "+String.format("%.1f", lidarminy));
		sendOSC("TO","/pf/maxy/label","Max Y: "+String.format("%.1f", lidarmaxy));
		sendOSC("TO","/pf/rotation/label","Rotation: "+String.format("%.1f", lidarRotation));
		Config.setFloat("video", "minx", minx);
		Config.setFloat("video", "maxx", maxx);
		Config.setFloat("video", "miny", miny);
		Config.setFloat("video", "maxy", maxy);
		makeCanvases();
	}

    synchronized public void locklidar(float onoff) {
	lockLIDARToVideo(0.0f);  // Distance to expand
    }

    // Adjust LIDAR bounds to be a fixed margin around video bounds
    	public void lockLIDARToVideo(float margin) {
	    lidarminx=minx-margin;
	    lidarminy=miny-margin;
	    lidarmaxx=maxx+margin;
	    lidarmaxy=maxy+margin;
	    sendOSC("FE","/pf/minx",lidarminx);
	    sendOSC("FE","/pf/maxx",lidarmaxx);
	    sendOSC("FE","/pf/miny",lidarminy);
	    sendOSC("FE","/pf/maxy",lidarmaxy);
	    logger.info("Set LIDAR to video bounds + " + margin + ": " + lidarminx + "," + lidarminy + " - " + lidarmaxx + "," + lidarmaxy);
	    resetcoords();
	}
    
	// Get pixels per meter
	public static float getPixelsPerMeter() {
		return Math.min(theTracker.canvas.width/getFloorSize().x, theTracker.canvas.height/getFloorSize().y);
	}
	
	// Get center of active area (in meters)
	public static PVector getFloorCenter() {
		return new PVector((minx+maxx)/2,(miny+maxy)/2);
	}
	
	// Get size of active area (in meters)
	public static PVector getFloorSize() {
		return new PVector(maxx-minx,maxy-miny);
	}

	// Get maximum floor dimension (in meters).
	public static float getFloorDimensionMax() {
		return Math.max(maxx - minx, maxy - miny);
	}

	// Get maximum floor dimension (in meters).
	public static float getFloorDimensionMin() {
		return Math.min(maxx - minx, maxy - miny);
	}
	
	synchronized public void pfstarted() {
		logger.entering(this.toString(), "pfstarted");
		logger.info("PF started");
	}

	synchronized public void pfstopped() {
		logger.info("PF stopped");
		people.clear();
	}

	void pfframe(int frame) {
		//logger.fine("Got frame "+frame);

		lastFrameReceived=frame;
	}

	void updateTO(boolean ledOn) {
		sendOSC("TO","/health/VD",ledOn?1f:0f);
		sendOSC("TO","/health/AL",ledOn&liveTick>0&liveTick-tick<500?1f:0f);  // Ableton is alive
		sendOSC("TO","/touchosc/fps",String.format("%.1f",avgFrameRate));
		sendOSC("TO","/video/bounds",drawBounds?1f:0f);
		sendOSC("TO","/video/borders",drawBorders?1f:0f);
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
//			logger.fine("Sending data for mouse person "+p.id);
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
//						logger.fine("Mouse "+id+" and "+id2+" grouped with distance "+dist);
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
			logger.fine("update:  samp="+sampnum+",elapsed="+elapsed+",id="+id+",pos=("+xpos+","+ypos+")"+",vel=("+xvelocity+","+yvelocity+")"+",axislength=("+majoraxis+","+minoraxis+")"+",channel="+channel);
		} */
		// NOTE: Need to map xvelocity,yvelocity before using them!

//		if (xpos<Tracker.lidarminx-1) {
//			logger.fine("Got xpos ("+xpos+") less than lidarminx ("+Tracker.lidarminx+")");
//		}
//		if (xpos>Tracker.lidarmaxx+1) {
//			logger.warning("Got xpos ("+xpos+") greater than lidarmaxx ("+Tracker.lidarmaxx+")");
//		}
//		if (ypos<Tracker.lidarminy-1) {
//			logger.warning("Got ypos ("+ypos+") less than lidarminy ("+Tracker.lidarminy+")");
//		}
//		if (ypos>Tracker.lidarmaxy+1) {
//			logger.warning("Got ypos ("+ypos+") greater than lidarmaxy ("+Tracker.lidarmaxy+"),");
//		}

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
		if (Tracker.lidarminx != minx) {
			Tracker.lidarminx=minx;
			resetcoords();
		}
	}
	public void pfsetminy(float miny) {  
		if (Tracker.lidarminy != miny) {
			Tracker.lidarminy=miny;
			resetcoords();
		}
	}
	public void pfsetmaxx(float maxx) {  
		if (Tracker.lidarmaxx!=maxx) {
			Tracker.lidarmaxx=maxx;
			resetcoords();
		}
	}
	public void pfsetmaxy(float maxy) { 
		if (Tracker.lidarmaxy != maxy) {
			Tracker.lidarmaxy=maxy;
			resetcoords();
		}
	}
	
	public void pfsetrotation(float deg) {
		if (Tracker.lidarRotation != deg) {
			Tracker.lidarRotation=deg;
			resetcoords();
		}
	}

	public void pfaligncorner(int unit, int cornerNumber, int numCorners, float x, float y, float wx, float wy) {
		//logger.fine("Corner "+cornerNumber+"/"+numCorners+" at "+x+", "+y);
		unit=unit-1;  // Switch to 0-based indexing
		if (alignCorners.length < unit+1)
			alignCorners=new PVector[unit+1][numCorners];
		if (alignCorners[unit].length!=numCorners) {
			//logger.fine("Resize alignCorners from "+alignCorners.length+" to "+numCorners);
			alignCorners[unit]=new PVector[numCorners];
		}
		if (cornerNumber >= 0)
			alignCorners[unit][cornerNumber]=new PVector(wx,wy);
	}
	

	public void pfgeo(int frame, int id, float centerDist, float otherDist, float existDist) {
		// Not implemented.
	}
	
	public void pfgroup(int frame, int gid, int gsize, float life, float centroidX, float centroidY, float diameter) {
		// Not implemented.
	}
	
	public void pfscanpt(int unit, int scanPt,float x, float y) {
		if (lidar.length<unit) {
			logger.config("Resizing lidar[] to "+(unit+1)+" entries");
			lidar=new PVector[unit][lidar[0].length];
		}
		unit=unit-1;  // Switch to 0-origin
		if (lidar[unit].length<scanPt+1) {
			logger.config("Resizing lidar["+unit+"] to "+(scanPt+1)+" entries");
			lidar[unit]=new PVector[scanPt+1];
		}
		lidar[unit][scanPt]=new PVector(x,y);
		//logger.finer("scanPt("+unit+", "+scanPt+", "+x+", "+y+" -> "+lidar[unit][scanPt]);
	}
	
	public void pfoutsiders(int group, int ndiv, int v1,  int v2,  int v3,  int v4) {
		logger.fine("Got /pf/outsiders " +group+", "+ndiv);
		LEDs.theLEDs.setOutsiderDivisions(ndiv);
		LEDs.theLEDs.setOutsiders(group, v1);
		LEDs.theLEDs.setOutsiders(group+1, v2);
		LEDs.theLEDs.setOutsiders(group+2, v3);
		LEDs.theLEDs.setOutsiders(group+3, v4);
	}
	
	public void cycle() {
		int newvis=currentvis;
		do {
			newvis=(newvis+1)%vis.length;
		} while (!vis[newvis].selectable);
		setapp(newvis);
//		Calendar cal=Calendar.getInstance();
//		int hour=cal.get(Calendar.HOUR_OF_DAY);
//		logger.fine("Autocycling hour = "+hour);
//		cycler.change(hour>=7 && hour <= 19);
	}
	
	synchronized public void pfsetnpeople(int n) {
		logger.fine("/pf/set/npeople: now have "+n+" people, size="+people.pmap.size());
		if (n==0)
			setapp(currentvis);   // Cause a reset
		if (n==0 && people.pmap.size()>0)
			if (autocycle)
				cycle();
//			else if (currentvis != getAppIndex("Bowie"))
//				setapp(getAppIndex("Navier"));
		
		people.setnpeople(n);  // Also clears positions
	}

	synchronized public void pfexit(int sampnum, float elapsed, int id) {
		logger.fine("exit: sampnum="+sampnum+", elapsed="+elapsed+", id="+id);
		people.exit(id);
		if (people.pmap.size()==0)
			if (autocycle)
				cycle();
//			else if (currentvis != getAppIndex("Bowie"))
//				setapp(getAppIndex("Navier"));
	}

	synchronized public void pfentry(int sampnum, float elapsed, int id, int channel) {
		add(id,channel);
		logger.fine("entry: sampnum="+sampnum+", elapsed="+elapsed+", id="+id+", channel="+channel+", color="+people.get(id).getcolor());
	}

	public void noteOn(int channel, int pitch, int velocity) {
		logger.fine("Got note on: channel="+channel+", pitch="+pitch+", velocity="+velocity);
	}

	public void noteOff(int channel, int pitch, int velocity) {
		logger.fine("Got note off: channel="+channel+", pitch="+pitch+", velocity="+velocity);
	}

	void controllerChange(int channel, int number, int value) {
		// Receive a controllerChange
		logger.fine("Got CC: channel="+channel+", CC="+number+", value="+value);
	}

	public boolean inBounds(PVector location) {
		// TODO Auto-generated method stub
		return location.x>=minx && location.x <=maxx && location.y >= miny && location.y <= maxy;
	}

}
