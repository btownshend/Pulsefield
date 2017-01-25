import java.awt.Color;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;

class VisualizerNavierOF extends VisualizerSyphon {
	float dissipation, velocityDissipation, tempDissipation, pressDissipation, gravityX, gravityY, limitVelocity; //  Parameters of model
	float alpha, legScale, saturation, brightness, density, temperature;  // Parameters of leg effects
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

		dissipation=0.999f;
		velocityDissipation=0.9f;
		pressDissipation=0.9f;
		tempDissipation=0.99f;
		gravityX=0f;
		gravityY=0f;
		alpha=128f;
		legScale=1.0f;
		saturation=1.0f;
		brightness=1.0f;
		temperature=10f;
		density=1f;
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
			dissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("velocityDissipation")) {
			velocityDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("tempDissipation")) {
			tempDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("pressDissipation")) {
			pressDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("saturation")) {
			saturation=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("brightness")) {
			brightness=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("alpha")) {
			alpha=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("legscale")) {
			legScale=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("temperature")) {
			temperature=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("density")) {
			density=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("gravityClear") ) {
			gravityX=0f; gravityY=0f;
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
		setTOValue("tempDissipation",Math.log10(1-tempDissipation),"%.2f");
		setTOValue("pressDissipation",Math.log10(1-pressDissipation),"%.2f");
		setTOValue("gravity/x",gravityX,"%.2f");
		setTOValue("gravity/y",gravityY,"%.2f");
		setTOValue("brightness",brightness,"%.2f");
		setTOValue("saturation",saturation,"%.2f");
		setTOValue("alpha",alpha,"%.2f");
		setTOValue("legscale",legScale,"%.2f");
		setTOValue("temperature",temperature,"%.2f");
		setTOValue("density",density,"%.2f");
		setOF("dissipation",dissipation);
		setOF("velocityDissipation",velocityDissipation);
		setOF("temperatureDissipation",tempDissipation);
		setOF("pressureDissipation",pressDissipation);
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

	void applyForce(int cellX, int cellY, double dx, double dy, float red, float green, float blue, float alpha, float radius, float temp, float dens) {
		OscMessage msg = new OscMessage("/navier/force");
		msg.add(cellX);
		msg.add(cellY);
		msg.add((float)dx);
		msg.add((float)dy);
		msg.add(red/255.0f);
		msg.add(green/255.0f);
		msg.add(blue/255.0f);
		msg.add(alpha);
		msg.add(radius);
		msg.add(temp);
		msg.add(dens);
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
			for (int l=0;l<pos.legs.length;l++) {
				Leg leg = pos.legs[l];
				int cellX = (int)( (-leg.getNormalizedPosition().x+1)*nwidth / 2);
				cellX=Math.max(0,Math.min(cellX,nwidth));
				int cellY = (int) ((leg.getNormalizedPosition().y+1)*nheight/ 2);
				cellY=Math.max(0,Math.min(cellY,nheight));

				double dx=leg.getNormalizedVelocity().x*nwidth/2; // In pixels/sec
				double dy=leg.getNormalizedVelocity().y*nheight/2;
				float radius=leg.getDiameterInMeters()/Tracker.getFloorSize().x*nwidth/2*legScale;
				int c=Color.HSBtoRGB(((pos.id*17+l*127)&0xff)/255.0f,saturation,brightness);
				
				PApplet.println("Leg "+l+": Cell="+cellX+","+cellY+", vel="+dx+","+dy+ ", radius="+radius+", color="+PApplet.hex(c));
				dx = (Math.abs(dx) > limitVelocity) ? Math.signum(dx) * limitVelocity : dx;
				dy = (Math.abs(dy) > limitVelocity) ? Math.signum(dy) * limitVelocity : dy;
				applyForce(cellX, cellY, dx, dy, parent.red(c), parent.green(c),parent.blue(c),alpha,radius,temperature,density);
			}
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

