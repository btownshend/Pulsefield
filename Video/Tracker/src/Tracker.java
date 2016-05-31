import java.awt.Color;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import codeanticode.syphon.SyphonServer;
import netP5.NetAddress;
import oscP5.OscMessage;
import oscP5.OscP5;
import oscP5.OscProperties;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PMatrix2D;
import processing.core.PMatrix3D;
import processing.core.PVector;
import processing.opengl.PGraphicsOpenGL;
import processing.opengl.PJOGL;


public class Tracker extends PApplet {
	/**
	 * 
	 */
	public static final String SVGDIRECTORY="../../../SVGFiles/";
	private static boolean present = false;
	private static boolean autocycle = true;
	private static boolean starting = true;   // Disable bad OSC messages before setup
	private static boolean genFrameMsgs = false;
	@SuppressWarnings("unused")
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
	public static final String visnames[]={"Pads","Navier","Tron","Grid","DDR","Poly","Voronoi","Guitar","Dot","CHucK","Proximity","Cows","Soccer","Menu","Visualizer","TestPattern"};
	public static boolean selectable[]={false,true,true,true,true,true,false,true,false,false,true,true,true,false,true,false};
//	public static boolean selectable[]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,true};
	String vispos[]={"5/1","5/2","5/3","5/4","5/5","4/1","4/2","4/3","4/4","4/5","3/1","3/2","3/3","3/4","3/5","2/1"};
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
	final static int CANVASWIDTH=1200;
	final static int CANVASHEIGHT=600;
	PGraphicsOpenGL canvas;
	Projector projectors[];
	Config jconfig;
	static PVector alignCorners[]=new PVector[0];
	static ProjCursor cursors[]=null;
	Map<String,Boolean> unhandled;
	PVector[] lidar = new PVector[381];
	PGraphicsOpenGL mask[];
	int pselect[];
	
	public void settings() {
		// If Tracker uses FX2D or P2D for renderer, then we can't do 3D and vortexRenderer will be blank!
		size(1280, 720, renderer);
		PJOGL.profile=1;    // Seems that this is needed for Syphon to function
		//pixelDensity(2);  // This breaks the Navier visualizer
	}

	public void setup() {
		configFile="/Users/bst/DropBox/Pulsefield/src/urlconfig.txt";
		jconfig=new Config("config.json");
		Config.setFloat("main", "test", 1.0f);
		Config.save(this);
		//Config.load(this);
		
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
		vis[15]=new VisualizerTestPattern(this);
		setapp(15);
		
		// Setup OSC handlers
		oscP5.plug(this, "pfframe", "/pf/frame");
		oscP5.plug(this, "pfupdate", "/pf/update");
		oscP5.plug(this, "pfbackground","/pf/background");
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
		oscP5.plug(visAbleton,  "songIncr", "/touchosc/song/incr");
		unhandled = new HashMap<String,Boolean>();
		canvas = (PGraphicsOpenGL) createGraphics(CANVASWIDTH, CANVASHEIGHT, renderer);
		projectors=new Projector[2];
		projectors[0] = new Projector(this,1,1920,1080);
		projectors[0].setPosition(0.0f, 0.0f, 1.5f);
		projectors[1] = new Projector(this,2,1920,1080);
		projectors[1].setPosition(2f, 3f, 1.5f);
		float mscale=4;	// How much smaller to make the masks
		mask=new PGraphicsOpenGL[projectors.length];
		for (int i=0;i<mask.length;i++) {
			mask[i]=(PGraphicsOpenGL) createGraphics((int)(CANVASWIDTH/mscale+0.5), (int)(CANVASHEIGHT/mscale+0.5),renderer);
			// Set for unmasked in case it is not used
			mask[i].beginDraw();
			mask[i].background(255);
			mask[i].endDraw();
		}
		pselect=new int[mask[0].width*mask[0].height];
		for (int i=0;i<pselect.length;i++)
			pselect[0]=0;  // Default to projector 0
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

	public void setcursor(int cursor, int ncursor, int proj, float x, float y) {
		PApplet.println("setcursor("+proj+","+cursor+","+ncursor+","+x+","+y+")");
		if (ncursor==0)
			cursors=null;
		else if (cursors==null || cursors.length != ncursor) {
			PApplet.println("setcursor: resizing to "+ncursor);
			cursors=new ProjCursor[ncursor];
		}
		if (ncursor>0) {
			assert(cursor<ncursor);
			cursors[cursor]=new ProjCursor(proj, x, y);
		}
	}
	
	public void setscreen2world(int proj, float x00, float x01, float x02, float x10, float x11, float x12, float x20, float x21, float x22) {
		PApplet.println("setscreen2world("+proj+","+x00+","+x01+"...)");
		if (proj<0 || proj>=projectors.length) {
			PApplet.println("setscreen2world: Bad projector number: "+proj);
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
		PApplet.println("setworld2screen("+proj+","+x00+","+x01+"...)");
		if (proj<0 || proj>=projectors.length) {
			PApplet.println("setworld2screen: Bad projector number: "+proj);
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
		PApplet.println("setprojection("+proj+":");
		mat.print();
		projectors[proj].setProjection(mat);
	}
	
	public void setpose(int proj, float x, float y, float z) {
		PVector vec=new PVector(x,y,z);
		PApplet.println("setpose("+proj+","+vec+")");
		projectors[proj].setPosition(x, y, z);
	}

	public void setcameraview(int proj, float x00, float x01, float x02, float x03, float x10, float x11, float x12, float x13, float x20, float x21, float x22, float x23) {
		PMatrix3D mat=new PMatrix3D();
		mat.set(x00,x01,x02,x03,
				x10,x11,x12,x13,
				x20,x21,x22,x23,
				0,0,0,1);
		PApplet.println("setcamerview("+proj+":");
		mat.print();
		projectors[proj].setCameraView(mat);
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

		canvas.beginDraw();
		// Transform so that coords for drawing are in meters
		canvas.resetMatrix();

		canvas.translate(canvas.width/2, canvas.height/2);   // center coordinate system to middle of canvas
		// The LIDAR uses a RH x-y axis (with origin top-center), but PGraphics uses a LH (with origin top-left)
		// So, we can flip the x-axis to get things aligned
		canvas.scale(-getPixelsPerMeter(),getPixelsPerMeter()); 
		canvas.translate(-getFloorCenter().x, -getFloorCenter().y);  // translate to center of new space

		float cscale=Math.min((width-2)*1f/canvas.width,(height-2)*1f/canvas.height); // Scaling of canvas to window
		//println("cscale="+cscale);
		if (mousePressed) {
			Person p=mousePeople.getOrCreate(mouseID,mouseID%16);
			//PVector sMousePos=normalizedToFloor(new PVector(mouseX*2f/width-1, mouseY*2f/height-1));
			float cmouseX=(mouseX/cscale-canvas.width/2)/getPixelsPerMeter()+(rawminx+rawmaxx)/2;
			float cmouseY=(mouseY/cscale-canvas.height/2)/getPixelsPerMeter()+(rawminy+rawmaxy)/2;
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

		vis[currentvis].draw(this, canvas,people);

		vis[currentvis].drawLaser(this,people);

		boolean drawBounds=true;   // True to overlay projector bounds
		if (drawBounds) {
			canvas.strokeWeight(0.025f);
			canvas.noFill();
			for (int i=0;i<projectors.length;i++) {
				canvas.stroke(i==0?255:0,i==1?255:0,i>1?255:0);
				canvas.beginShape();
				for (int j=0;j<projectors[i].bounds.length;j++) {
					PVector p=projectors[i].bounds[j];
					canvas.vertex(-p.x,p.y);
					//PApplet.println("vertex("+projectors[i].bounds[j]+") -> "+canvas.screenX(p.x,p.y)+","+canvas.screenY(p.x, p.y));
				}
				canvas.endShape(CLOSE);
			}
		}
		
		canvas.endDraw();
		// Syphon setup, requires OpenGL renderer (not FX2D?)
		if (useSyphon && server==null)
			server = new SyphonServer(this, "Tracker");
		
		if (server != null) {
			server.sendImage(canvas);
		}
		buildMasks(canvas);
		projectors[0].render(canvas,mask[0]);
		projectors[1].render(canvas,mask[1]);
	

		imageMode(CENTER);
		// Canvas is RH, so need to flip it back to draw on main window (which is LH)
		pushMatrix();
		translate(width/2,height/2);
		scale(-1,1);
		translate(-width/2,-height/2);
		image(canvas, width/2, height/2, canvas.width*cscale, canvas.height*cscale);
		popMatrix();


		// Use top-left, top-right corners for projector images
		float pfrac = 0.25f;  // Use this much of the height of the window for projs
		float pheight=this.height*pfrac;
		float pwidth=pheight*projectors[0].pcanvas.width/projectors[0].pcanvas.height;  // preserve aspect
		stroke(255,0,0);
		strokeWeight(2);
		rect(0, 0, pwidth, pheight);
		imageMode(CORNER);
		image(projectors[0].pcanvas, 1, 1, pwidth-2, pheight-2);
		
		pwidth=pheight*projectors[1].pcanvas.width/projectors[1].pcanvas.height; 
		rect(width-pwidth, 0, pwidth, pheight);
		image(projectors[1].pcanvas, width-pwidth+1, 1f, pwidth-2, pheight-2);
		
		boolean drawMasks = true;
		if (drawMasks) {
			// Draw masks on screen
			imageMode(CORNER);
			rect(0, height-mask[0].height-2, mask[0].width+2, mask[0].height+2);
			image(mask[0],1,height-mask[0].height-1);
			rect(width-mask[0].width-2, height-mask[1].height-2, mask[1].width+2, mask[1].height+2);
			image(mask[1],width-mask[0].width-1,height-mask[1].height-1);
		}
		//SyphonTest.draw(this);
		Config.saveIfModified(this);   // Save if modified
	}

	private void buildMasks(PGraphicsOpenGL canvas) {
		for (int i=0;i<mask.length;i++) {
			mask[i].beginDraw();
			// Transform so that coords for drawing are in meters
			mask[i].resetMatrix();
			mask[i].translate(mask[i].width/2, mask[i].height/2);   // center coordinate system to middle of canvas
			mask[i].scale(-getPixelsPerMeter()*mask[i].width/canvas.width, -getPixelsPerMeter()*mask[i].height/canvas.height); // FIXME: Unclear why, but need to flip y here
			mask[i].translate(-getFloorCenter().x, -getFloorCenter().y);  // translate to center of new space
//			println("[0,0]->canvas "+canvas.screenX(0, 0)+","+canvas.screenY(0,0));
//			println("[0,0]->mask "+mask[i].screenX(0, 0)+","+mask[i].screenY(0,0));
			mask[i].blendMode(PConstants.REPLACE);
			mask[i].imageMode(PConstants.CORNER);
			mask[i].ellipseMode(PConstants.CENTER);
			mask[i].background(0);  // default, nothing drawn
			mask[i].fill(255);      // Can draw anywhere within bounds
			mask[i].beginShape();
			for (int j=0;j<projectors[i].bounds.length;j++) {
				PVector p=projectors[i].bounds[j];
				mask[i].vertex(-p.x,p.y);
				//PApplet.println("vertex("+projectors[i].bounds[j]+") -> "+canvas.screenX(p.x,p.y)+","+canvas.screenY(p.x, p.y));
			}
			mask[i].endShape(CLOSE);
//			mask[i].fill(127);
//			mask[i].rect(i, 1+i, 1, 1);
//			mask[i].fill(0);
//			mask[i].rect(i, 1+i, 0.5f, 0.5f);
			PVector projPos=projectors[i].pos;projPos.z=0;
			mask[i].stroke(1);  // Don't draw here, but not as insistent as being out of bounds
			mask[i].strokeWeight(0.5f);
			for (Person ps: people.pmap.values()) {  
				PVector pos=ps.getOriginInMeters();
				PVector vec=PVector.sub(pos, projPos); vec.normalize();
				println("shadow of "+pos+" from proj@ "+projPos+": vec="+vec);
				vec.mult(6);  // Draw a line that will run past edge
				mask[i].line(pos.x, pos.y, pos.x+vec.x, pos.y+vec.y);
				//mask[i].ellipse(pos.x, pos.y, 0.5f, 0.5f);
			}
			mask[i].endDraw();
			mask[i].loadPixels();
		}
		
		int maskcnt[]=new int[pselect.length];
		// pselect is a persistent map of which projector gets which pixel
		for (int i=0;i<pselect.length;i++) {
			if ((mask[pselect[i]].pixels[i]&0xff)<255) {
				for (int j=0;j<mask.length;j++) {
					if ((mask[j].pixels[i]&0xff) > (mask[pselect[i]].pixels[i]&0xff))
						pselect[i]=j;  // Switch to a better projector
				}
			}
			maskcnt[pselect[i]]+=1;
		}
		
		PApplet.println("Mask has projector 0: "+maskcnt[0]+", 1: "+maskcnt[1]);
		// Now bring those back to the masks
//		int fullon=0xffffffff;
//		int fulloff=0xff000000;
//		for (int j=0;j<mask.length;j++)
//			for (int i=0;i<pselect.length;i++)
//				mask[j].pixels[i]=(pselect[i]==j)?fullon:fulloff;
		
		
//		// Spread the masks
//		int zn=0;
//		for (int n=0;n<500;n++) {
//			int n0=0, n1=0;
//			for (int i=n%2;i<mask[0].width;i+=2)  // Interleave steps to avoid propagating asymmetrically
//				for (int j=n%3;j<mask[0].height;j+=2) {
//					int ind=j*mask[0].width+i;
//					if (p[ind]==127) {// Indeterminate
//						//PApplet.println("len="+p.length+", j="+j+", ind="+ind);
//						int sum=((i<mask[0].width-1)?p[ind+1]:127)+
//								((i>0)?p[ind-1]:127)+
//								((j<mask[0].height-1)?p[ind+mask[0].width]:127)+
//								((j>0)?p[ind-mask[0].width]:127);
//						if (sum>127*4) {
//							p[ind]=255;
//							n0+=1;
//						} else if (sum<127*4) {
//							p[ind]=0;
//							n1+=1;
//						}
//					}
//				}
//			println("n="+n+": Dilated "+n0+","+n1+" pixels");
//			// Keep going until there is no change for 6 cycles (repeat of offsets)
//			if (n0+n1 == 0)
//				zn+=1;
//			else
//				zn=0;
//			if (zn==6)
//				break;
//		}
		// Blur it
//		mask[0].filter(BLUR);
//		// Make mask[1] the inverse
//		for (int i=0;i<p.length;i++)
//			mask[1].pixels[i]=255-p[i];
		
		for (int i=0;i<mask.length;i++)
			mask[i].updatePixels();
		
//		mask[0].loadPixels();
//		mask[1].loadPixels();
//		for (int i=0;i<mask[0].width;i++)
//			for (int j=0;j<mask[0].height;j++) {
//				int alpha=Math.min(255,Math.max(0,((int)(255*((i*1.0/mask[0].width)-0.5)*3)+127)));
//				mask[0].pixels[j*mask[0].width+i]=(alpha<<24)+(alpha<<16)+(alpha<<8)+alpha;
//				alpha=255-alpha;
//				mask[1].pixels[j*mask[1].width+i]=(alpha<<24)+(alpha<<16)+(alpha<<8)+alpha;
////				if (j==100 && i==100)
////					PApplet.println("alpha(100,100)="+PApplet.hex(alpha));
//			}
//		mask[0].updatePixels();
//		mask[1].updatePixels();
		//PApplet.println("m0="+PApplet.hex(mask[0].get(100, 100))+", m1="+PApplet.hex(mask[1].get(100, 100)));
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
			PApplet.main(new String[] { "--present","Tracker"});
		else
			PApplet.main(new String[] {"--display=2","Tracker" });
	}

	/* incoming osc message are forwarded to the oscEvent method. */
	synchronized public void oscEvent(OscMessage theOscMessage) {
		//PApplet.println("Got message:"+theOscMessage.toString());
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
		} else if (theOscMessage.addrPattern().startsWith("/proj")) {
			String pattern=theOscMessage.addrPattern();
			String components[]=pattern.split("/");
			PApplet.println("num comp="+components.length+", comp2="+components[2]);
			if (components.length>=3) {
				int pnum=Integer.parseInt(components[2]);
				if (pnum>=1 && pnum<=projectors.length)
					projectors[pnum-1].handleMessage(theOscMessage);
				else
					PApplet.println("Bad projector number: "+pnum);
			}
		} else if (theOscMessage.addrPattern().startsWith("/video/navier")) {
			visNavier.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/video/ddr")) {
			visDDR.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/midi/pgm")) {
			synth.handleMessage(theOscMessage);
		} else if (theOscMessage.addrPattern().startsWith("/pf/set")) {
			// PApplet.println("Unhandled set message: "+theOscMessage.addrPattern());
		} else if (theOscMessage.addrPattern().startsWith("/vis/")) {
			// PApplet.println("Unhandled vis message: "+theOscMessage.addrPattern());
		} else if (!unhandled.containsKey(theOscMessage.addrPattern())) {
			PApplet.print("Received an unhandled OSC message: ");
			theOscMessage.print();
			unhandled.put(theOscMessage.addrPattern(),true);		
		}  /* print the address pattern and the typetag of the received OscMessage */
	}

	public static PVector mapVelocity(PVector velInMetersPerSecond) {
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
		PVector mid=getFloorCenter();
		PVector sz=getFloorSize();
		PVector result=new PVector(mapped.x*sz.x/2+mid.x,mapped.y*sz.y/2+mid.y);
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

	// Get pixels per meter
	public static float getPixelsPerMeter() {
		return Math.min(CANVASWIDTH/getFloorSize().x, CANVASHEIGHT/getFloorSize().y);
	}
	
	// Get center of active area (in meters)
	public static PVector getFloorCenter() {
		return new PVector((rawminx+rawmaxx)/2,(rawminy+rawmaxy)/2);
	}
	
	// Get size of active area (in meters)
	public static PVector getFloorSize() {
		return new PVector(rawmaxx-rawminx,rawmaxy-rawminy);
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

	public void pfaligncorner(int cornerNumber, int numCorners, float x, float y) {
		//PApplet.println("Corner "+cornerNumber+"/"+numCorners+" at "+x+", "+y);

		if (alignCorners.length!=numCorners) {
			//PApplet.println("Resize alignCorners from "+alignCorners.length+" to "+numCorners);
			alignCorners=new PVector[numCorners];
		}
		if (cornerNumber >= 0)
			alignCorners[cornerNumber]=new PVector(x,y);
	}
	

	public void pfgeo(int frame, int id, float centerDist, float otherDist, float existDist) {
		// Not implemented.
	}
	
	public void pfgroup(int frame, int gid, int gsize, float life, float centroidX, float centroidY, float diameter) {
		// Not implemented.
	}

	public void pfbackground(int scanPt,int nrange,float angle,float backRange,float currRange) {
		if (lidar.length != nrange)
			lidar=new PVector[nrange];
		lidar[scanPt]=new PVector(currRange*cos(angle*PI/180),currRange*sin(angle*PI/180));
		//PApplet.println("background("+scanPt,", "+nrange+", "+angle+", "+backRange+", "+currRange+") -> "+lidar[scanPt]);
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

