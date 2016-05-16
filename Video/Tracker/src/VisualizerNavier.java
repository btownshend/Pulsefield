import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

class VisualizerNavier extends Visualizer {
	NavierStokesSolver fluidSolver;
	double visc, diff, limitVelocity;
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
	Synth synth;
	MusicVisLaser mvl;
	final int downSample=1;   // amount to downsample fluid image
	
	VisualizerNavier(Tracker parent, Synth synth) {
		super();
		fluidSolver = new NavierStokesSolver();

		visc = 0.001;
		diff = 3.0e-4;
		scale = 0.3;

		limitVelocity = 200;
		// iao = loadImage("IAO.jpg");
		//background(iao);

		setTO();
		stats();
		
		this.synth=synth;
		mvl=new MusicVisLaser(parent.fourier, MusicVisLaser.Modes.POLYGON);
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
		
		if (components.length<3 || !components[2].equals("navier")) 
			PApplet.println("Navier: Expected /video/navier messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("viscosity")) {
			visc=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("diffusion")) {
			diff=Math.pow(10f,msg.get(0).floatValue());
		} else if (components.length==4 && components[3].equals("scale")) {
			scale=msg.get(0).floatValue();
		} else 
			PApplet.println("Unknown Navier Message: "+msg.toString());
		PApplet.println("visc="+visc+", diff="+diff+", scale="+scale);
		setTO();
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/navier/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/video/navier/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}
	
	public void setTO() {
		setTOValue("scale",scale,"%.2f");
		setTOValue("diffusion",Math.log10(diff),"%.2f");
		setTOValue("viscosity",visc,"%.4f");
	}
	
	public void stats() {
		long elapsed=System.nanoTime()-statsLast;
		PApplet.println("Total="+elapsed/1e6+"msec , Tick="+statsTick*100f/elapsed+"%, Step="+statsStep*100f/elapsed+"%, Update="+statsUpdate*100f/elapsed+"%");
		statsLast = System.nanoTime();
		statsTick=0;
		statsStep=0;
		statsUpdate=0;
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p, PVector wsize) {
//		Don't call parent.draw since it draws the defaults border and fills the background
		if (buffer==null) {
			buffer = new PImage(g.width/downSample, g.height/downSample);
		}
		if (p.pmap.isEmpty()) {
			g.background(0, 0, 0);  
			g.colorMode(PConstants.RGB, 255);
			drawWelcome(g,wsize);
			return;
		}

		double dt = 1 / t.frameRate;
		long t1 = System.nanoTime();
		fluidSolver.tick(dt, visc, diff);
		long t2 = System.nanoTime();
		fluidCanvasStep(g);
		long t3 = System.nanoTime();
		statsTick += t2-t1;
		statsStep += t3-t2;

		g.strokeWeight(7);
		g.colorMode(PConstants.HSB, 255);
		bordercolor = g.color(rainbow, 255, 255);
		rainbow++;
		rainbow = (rainbow > 255) ? 0 : rainbow;

		drawBorders(g, true, wsize, 7, bordercolor, 127);

		g.ellipseMode(PConstants.CENTER);
		for (Person ps: p.pmap.values()) {  
			int c=ps.getcolor();
			g.fill(c,255);
			g.stroke(c,255);
			//PApplet.println("groupsize="+ps.groupsize+" ellipse at "+ps.origin.toString());
			
			float sz=5;
			if (ps.groupsize > 1)
				sz=20*ps.groupsize;
			g.ellipse((ps.getNormalizedPosition().x+1)*wsize.x/2, (ps.getNormalizedPosition().y+1)*wsize.y/2, sz, sz);
		}
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
		int n = NavierStokesSolver.N;
		for (Person pos: p.pmap.values()) {
			//PApplet.println("update("+p.channel+"), enabled="+p.enabled);
			int cellX = (int)( (pos.getNormalizedPosition().x+1)*n / 2);
			cellX=Math.max(0,Math.min(cellX,n));
			int cellY = (int) ((pos.getNormalizedPosition().y+1)*n/ 2);
			cellY=Math.max(0,Math.min(cellY,n));
			double dx=-pos.getVelocityInMeters().x/parent.frameRate*10;
			double dy=pos.getVelocityInMeters().y/parent.frameRate*10;
			//PApplet.println("Cell="+cellX+","+cellY+", dx="+dx+", dy="+dy);

			dx = (Math.abs(dx) > limitVelocity) ? Math.signum(dx) * limitVelocity : dx;
			dy = (Math.abs(dy) > limitVelocity) ? Math.signum(dy) * limitVelocity : dy;
			fluidSolver.applyForce(cellX, cellY, dx, dy);
		}
		statsUpdate += System.nanoTime()-t1;
		
	}

	private void fluidCanvasStep(PGraphics g) {
		double widthInverse = 1.0 / g.width;
		double heightInverse = 1.0 / g.height;

		long t1=System.nanoTime();
		g.loadPixels();
		long t2=System.nanoTime();
		for (int y = 0; y < g.height; y+=downSample) {
			for (int x = 0; x < g.width; x+=downSample) {
				double u = (x+0.5) * widthInverse;
				double v = (y+0.5) * heightInverse;

				double warpedPosition[] = fluidSolver.getInverseWarpPosition(u, v, 
						scale);

				double warpX = warpedPosition[0];
				double warpY = warpedPosition[1];

				warpX *= g.width;
				warpY *= g.height;
				warpX-=0.5;
				warpY-=0.5;
				int collor = getSubPixel(g,warpX, warpY);
				//int collor=parent.pixels[((int)warpX)+((int)warpY)*parent.width];
				buffer.set(x/downSample, y/downSample, collor);
			}
		}
		g.imageMode(PConstants.CORNER);
		g.tint(255);
		g.image(buffer,0,0,g.width,g.height);
	}

	public int getSubPixel(PGraphics parent, double warpX, double warpY) {
		if (warpX < 0 || warpY < 0 || warpX > parent.width - 1 || warpY > parent.height - 1) {
			return bordercolor;
		}
		int x = (int) Math.floor(warpX);
		int y = (int) Math.floor(warpY);
		double u = warpX - x;
		double v = warpY - y;

		y = PApplet.constrain(y, 0, parent.height - 2);
		x = PApplet.constrain(x, 0, parent.width - 2);

//		int indexTopLeft = x + y * parent.width;
//		int indexTopRight = x + 1 + y * parent.width;
//		int indexBottomLeft = x + (y + 1) * parent.width;
//		int indexBottomRight = x + 1 + (y + 1) * parent.width;

		int cTL = parent.pixels[x + y * parent.width];
		int cTR = parent.pixels[x + 1 + y * parent.width];
		int cBL = parent.pixels[x + (y + 1) * parent.width];
		int cBR = parent.pixels[x + 1 + (y + 1) * parent.width];
		int color=0xff000000;
		for (int bit=0;bit<24;bit+=8)
			color|=((int)(((cTL>>bit)&0xff)*(1-u)*(1-v)+((cTR>>bit)&0xff)*u*(1-v)+((cBL>>bit)&0xff)*(1-u)*v+((cBR>>bit)&0xff)*u*v+0.5))<<bit;
		return color;
//		try {
//			int lc= lerpColor(parent, lerpColor(parent, parent.pixels[indexTopLeft], parent.pixels[indexTopRight], 
//					(float) u), lerpColor(parent, parent.pixels[indexBottomLeft], 
//							parent.pixels[indexBottomRight], (float) u), (float) v);
//			if (lc!=color) {
//				PApplet.println(String.format("old way=0x%x,  new way=0x%x\n", lc, color));
//			}
//		} 
//		catch (Exception e) {
//			System.out.println("error caught trying to get color for pixel position "
//					+ x + ", " + y);
//			return bordercolor;
//		}

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
	
	@Override
	public  void drawLaser(PApplet parent, People p) {
		// Delegate to the mvl
		mvl.drawLaser(parent, p);
	}
}

