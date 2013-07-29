import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;
import processing.core.PVector;


class VisualizerNavier extends Visualizer {
	NavierStokesSolver fluidSolver;
	double visc, diff, limitVelocity;
	int oldMouseX = 1, oldMouseY = 1;
	private int bordercolor;
	private double scale;
	PImage buffer;
	PImage iao;
	int rainbow = 0;
	long statsLast = 0;
	long statsTick = 0;
	long statsStep = 0;
	long statsUpdate = 0;

	VisualizerNavier(PApplet parent) {
		super();
		fluidSolver = new NavierStokesSolver();
		buffer = new PImage(parent.width, parent.height);

		visc = 0.001;
		diff = 3.0;
		scale = 0.3;

		limitVelocity = 200;
		// iao = loadImage("IAO.jpg");
		//background(iao);
		parent.colorMode(PApplet.HSB, 255);
		bordercolor = parent.color(0, 255, 255);
		parent.strokeWeight(7);
	}

	public void handleMessage(OscMessage msg) {
		PApplet.println("Navier message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		if (!components[1].equals("navier")) 
			PApplet.println("Navier: Expected /navier messages, got "+msg.toString());
		else if (components.length==3 && components[2].equals("viscosity")) {
			visc=msg.get(0).floatValue();
		} else if (components.length==3 && components[2].equals("diffusion")) {
			diff=msg.get(0).floatValue();
		} else if (components.length==3 && components[2].equals("scale")) {
			scale=msg.get(0).floatValue();
		} else 
			PApplet.println("Unknown Navier Message: "+msg.toString());
		PApplet.println("visc="+visc+", diff="+diff+", scale="+scale);
	}

	public void stats() {
		long elapsed=System.nanoTime()-statsLast;
		PApplet.println("Total="+elapsed/1e6+"msec , Tick="+statsTick*100f/elapsed+"%, Step="+statsStep*100f/elapsed+"%, Update="+statsUpdate*100f/elapsed+"%");
		statsLast = System.nanoTime();
		statsTick=0;
		statsStep=0;
		statsUpdate=0;
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		double dt = 1 / parent.frameRate;
		long t1 = System.nanoTime();
		fluidSolver.tick(dt, visc, diff);
		long t2 = System.nanoTime();
		fluidCanvasStep(parent);
		long t3 = System.nanoTime();
		statsTick += t2-t1;
		statsStep += t3-t2;

		parent.colorMode(PConstants.HSB, 255);
		bordercolor = parent.color(rainbow, 255, 255);
		rainbow++;
		rainbow = (rainbow > 255) ? 0 : rainbow;

		parent.stroke(bordercolor);
		parent.fill(0,0,0,0);
		drawBorders(parent, true, wsize);
		parent.ellipseMode(PConstants.CENTER);
		for (Position ps: p.positions.values()) {  
			int c=ps.getcolor(parent);
			parent.fill(c,100);
			parent.stroke(c,100);
			parent.ellipse(ps.origin.x, ps.origin.y, 3, 3);
		}
	}

	public void update(PApplet parent, Positions p) {
		long t1=System.nanoTime();
		int n = NavierStokesSolver.N;
		for (Position pos: p.positions.values()) {
			//PApplet.println("update("+p.channel+"), enabled="+p.enabled);
			int cellX = (int)( (pos.origin.x+1)*n / 2);
			cellX=Math.max(0,Math.min(cellX,n));
			int cellY = (int) ((pos.origin.y+1)*n/ 2);
			cellY=Math.max(0,Math.min(cellY,n));
			double dx=pos.avgspeed.x/parent.frameRate*100;
			double dy=pos.avgspeed.y/parent.frameRate*100;
			//PApplet.println("Cell="+cellX+","+cellY+", dx="+dx+", dy="+dy);

			dx = (Math.abs(dx) > limitVelocity) ? Math.signum(dx) * limitVelocity : dx;
			dy = (Math.abs(dy) > limitVelocity) ? Math.signum(dy) * limitVelocity : dy;
			fluidSolver.applyForce(cellX, cellY, dx, dy);
		}
		statsUpdate += System.nanoTime()-t1;
	}

	private void fluidCanvasStep(PApplet parent) {
		double widthInverse = 1.0 / parent.width;
		double heightInverse = 1.0 / parent.height;

		parent.loadPixels();
		for (int y = 0; y < parent.height; y++) {
			for (int x = 0; x < parent.width; x++) {
				double u = x * widthInverse;
				double v = y * heightInverse;

				double warpedPosition[] = fluidSolver.getInverseWarpPosition(u, v, 
						scale);

				double warpX = warpedPosition[0];
				double warpY = warpedPosition[1];

				warpX *= parent.width;
				warpY *= parent.height;

				int collor = getSubPixel(parent,warpX, warpY);
				//int collor=parent.pixels[((int)warpX)+((int)warpY)*parent.width];
				buffer.set(x, y, collor);
			}
		}
		parent.background(buffer);
	}

	public int getSubPixel(PApplet parent, double warpX, double warpY) {
		if (warpX < 0 || warpY < 0 || warpX > parent.width - 1 || warpY > parent.height - 1) {
			return bordercolor;
		}
		int x = (int) Math.floor(warpX);
		int y = (int) Math.floor(warpY);
		double u = warpX - x;
		double v = warpY - y;

		y = PApplet.constrain(y, 0, parent.height - 2);
		x = PApplet.constrain(x, 0, parent.width - 2);

		int indexTopLeft = x + y * parent.width;
		int indexTopRight = x + 1 + y * parent.width;
		int indexBottomLeft = x + (y + 1) * parent.width;
		int indexBottomRight = x + 1 + (y + 1) * parent.width;

		try {
			return lerpColor(parent, lerpColor(parent, parent.pixels[indexTopLeft], parent.pixels[indexTopRight], 
					(float) u), lerpColor(parent, parent.pixels[indexBottomLeft], 
							parent.pixels[indexBottomRight], (float) u), (float) v);
		} 
		catch (Exception e) {
			System.out.println("error caught trying to get color for pixel position "
					+ x + ", " + y);
			return bordercolor;
		}
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

