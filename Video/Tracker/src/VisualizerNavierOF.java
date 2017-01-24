import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;

class VisualizerNavierOF extends VisualizerSyphon {
	double dissipation, velocityDissipation, gravityX, gravityY, limitVelocity;
	int oldMouseX = 1, oldMouseY = 1;
	private int bordercolor;
	private double scale;
	PImage buffer=null;
	PImage iao;
	int rainbow = 0;
	long statsLast = 0;
	long statsTick = 0;
	long statsStep = 0;
	long statsUpdate = 0;
	long statsU1=0;
	long statsU2=0;
	long statsU3=0;
	long statsU4=0;
	Synth synth;
	//MusicVisLaser mvl;
	final int downSample=2;   // amount to downsample fluid image
	
	VisualizerNavierOF(Tracker parent, Synth synth, String appName, String serverName) {
		super(parent, appName, serverName);

		dissipation=0.99;
		velocityDissipation=0.99;
		gravityX=0;
		gravityY=-0.98;

		limitVelocity = 200;
		// iao = loadImage("IAO.jpg");
		//background(iao);

		setTO();
		stats();
		
		this.synth=synth;
		//mvl=new MusicVisLaser(parent.fourier, MusicVisLaser.Modes.POLYGON);
	}

	@Override
	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		Ableton.getInstance().setTrackSet("Navier");
	}

	@Override
	public void stop() {
		super.stop();
	}


	public void handleMessage(OscMessage msg) {
		PApplet.println("Navier message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		
		if (components.length<3 || !components[2].equals("navierOF")) 
			PApplet.println("Navier: Expected /video/navierOF messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("dissipation")) {
			dissipation=1-Math.pow(10f,msg.get(0).floatValue());
		} else if (components.length==4 && components[3].equals("velocityDissipation")) {
			velocityDissipation=1-Math.pow(10f,msg.get(0).floatValue());
		} else if (components.length==5 && components[3].equals("gravity") && components[4].equals("x")) {
			gravityX=msg.get(0).floatValue();
		} else if (components.length==5 && components[3].equals("gravity") && components[4].equals("y")) {
			gravityY=msg.get(0).floatValue();
		} else 
			PApplet.println("Unknown NavierOF Message: "+msg.toString());
		PApplet.println("dissipation="+dissipation+", velocityDissipation="+velocityDissipation+", gravity="+gravityX+","+gravityY);
		setTO();
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/navierOF/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/video/navierOF/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}
	
	private void setOF(String name, double value) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add((float)value);
		OFOSC.getInstance().sendMessage(set);
	}
	
	private void setOF(String name, double v1, double v2) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add((float)v1);
		set.add((float)v2);
		OFOSC.getInstance().sendMessage(set);
	}
	
	public void setTO() {
		setTOValue("scale",scale,"%.2f");
		setTOValue("dissipation",Math.log10(1-dissipation),"%.2f");
		setTOValue("velocityDissipation",Math.log10(1-velocityDissipation),"%.2f");
		setTOValue("gravity/x",gravityX,"%.2f");
		setTOValue("gravity/y",gravityY,"%.2f");
		setOF("dissipation",dissipation);
		setOF("velocityDissipation",velocityDissipation);
		setOF("gravity",gravityX,gravityY);
	}
	
	public void stats() {
		long elapsed=System.nanoTime()-statsLast;
		PApplet.println("Total="+elapsed/1e6+"msec , Tick="+statsTick*100f/elapsed+"%, Step="+statsStep*100f/elapsed+"%, Update="+statsUpdate*100f/elapsed+"%");
		PApplet.println("U=",statsU1*100f/elapsed,", ",statsU2*100f/elapsed,", ",statsU3*100f/elapsed,", ",statsU4*100f/elapsed);
		statsLast = System.nanoTime();
		statsTick=0;
		statsStep=0;
		statsUpdate=0;
		statsU1=0;
		statsU2=0;
		statsU3=0;
		statsU4=0;
	}

	void applyForce(int cellX, int cellY, double dx, double dy, float red, float green, float blue, float radius) {
		OscMessage msg = new OscMessage("/navier/force");
		msg.add(cellX);
		msg.add(cellY);
		msg.add((float)dx);
		msg.add((float)dy);
		msg.add(red/255.0f);
		msg.add(green/255.0f);
		msg.add(blue/255.0f);
		msg.add(radius);
		//PApplet.println("red="+(red/255.0)+", green="+(green/255.0)+", blue="+(blue/255.0));
		OFOSC.getInstance().sendMessage(msg);
	}
	
	@Override
	public void update(PApplet parent, People p) {
		Ableton.getInstance().updateMacros(p);
		for (Person pos: p.pmap.values()) {
			//PApplet.println("ID "+pos.id+" avgspeed="+pos.avgspeed.mag());
			if (pos.isMoving())
				synth.play(pos.id,pos.channel+35,127,480,pos.channel);
		}
		long t1=System.nanoTime();
		int nwidth=1024, nheight=1024;
		if (canvas!=null) {
			nwidth=canvas.width;
			nheight=canvas.height;
		}
		for (Person pos: p.pmap.values()) {
			//PApplet.println("update("+p.channel+"), enabled="+p.enabled);
			int cellX = (int)( (-pos.getNormalizedPosition().x+1)*nwidth / 2);
			cellX=Math.max(0,Math.min(cellX,nwidth));
			int cellY = (int) ((pos.getNormalizedPosition().y+1)*nheight/ 2);
			cellY=Math.max(0,Math.min(cellY,nheight));
			
			double dx=pos.getNormalizedVelocity().x*nwidth/2/parent.frameRate;
			double dy=pos.getNormalizedVelocity().y*nheight/2/parent.frameRate;
			//PApplet.println("Cell="+cellX+","+cellY+", dx="+dx+", dy="+dy);

			dx = (Math.abs(dx) > limitVelocity) ? Math.signum(dx) * limitVelocity : dx;
			dy = (Math.abs(dy) > limitVelocity) ? Math.signum(dy) * limitVelocity : dy;
			applyForce(cellX, cellY, dx, dy, parent.red(pos.getcolor()), parent.green(pos.getcolor()),parent.blue(pos.getcolor()),20.0f);
		}
		if (p.pmap.isEmpty()) {
		// Keep it moving
			//applyForce((int)(Math.random()*nwidth), (int)(Math.random()*nheight), limitVelocity/8*(Math.random()*2-1), limitVelocity/8*(Math.random()*2-1));
		}
		statsUpdate += System.nanoTime()-t1;
		//stats();
	}

	
	public int lerpColor(PApplet parent, int c1, int c2, float l) {
		parent.colorMode(PConstants.RGB, 255);
		float r1 = parent.red(c1)+0.5f;
		float g1 = parent.green(c1)+0.5f;
		float b1 = parent.blue(c1)+0.5f;

		float r2 = parent.red(c2)+0.5f;
		float g2 = parent.green(c2)+0.5f;
		float b2 = parent.blue(c2)+0.5f;

		return parent.color( PApplet.lerp(r1, r2, l), PApplet.lerp(g1, g2, l), PApplet.lerp(b1, b2, l) );
	}
	
}

