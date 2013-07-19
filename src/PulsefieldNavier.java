import java.util.Map;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PImage;


class PulsefieldNavier extends Pulsefield {
	NavierStokesSolver fluidSolver;
	double visc, diff, limitVelocity;
	int oldMouseX = 1, oldMouseY = 1;
	private int bordercolor;
	private double scale;
	PImage buffer;
	PImage iao;
	int rainbow = 0;

	PulsefieldNavier(PApplet parent) {
		super(parent);
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

	synchronized void draw() {
		updateSimulation();
		double dt = 1 / parent.frameRate;
		fluidSolver.tick(dt, visc, diff);
		fluidCanvasStep();

		parent.colorMode(PConstants.HSB, 255);
		bordercolor = parent.color(rainbow, 255, 255);
		rainbow++;
		rainbow = (rainbow > 255) ? 0 : rainbow;

		drawBorders();
	}

	private void drawBorders() {
		parent.stroke(bordercolor);
		parent.line(0, 0, parent.width-1, 0);
		parent.line(0, 0, 0, parent.height-1);
		parent.line(parent.width-1, 0, parent.width-1, parent.height-1);
		parent.line(0, parent.height-1, parent.width-1, parent.height-1);

		parent.fill(bordercolor);
		for (Map.Entry<Integer,Position> me: positions.entrySet()) {
			Position ps=me.getValue();  
			int id=(int)me.getKey();
			int c=getcolor(id);
			parent.fill(c);
			parent.stroke(c);;
			parent.ellipse(ps.origin.x, ps.origin.y, 10, 10);
		}
	}


	void updateSimulation() {
		int n = NavierStokesSolver.N;

		for (Position p: positions.values()) {
			if (p.enabled) {
				int cellX = (int)( p.origin.x*n / parent.width);
				int cellY = (int) (p.origin.y*n/parent.height);
				if (cellX<0 || cellX>=n)
					assert(false);
				if (cellY<0 || cellY>=n)
					assert(false);			
				double dx=p.avgspeed.x/parent.width*20;
				double dy=p.avgspeed.y/parent.height*20;

				dx = (Math.abs(dx) > limitVelocity) ? Math.signum(dx) * limitVelocity : dx;
				dy = (Math.abs(dy) > limitVelocity) ? Math.signum(dy) * limitVelocity : dy;

				fluidSolver.applyForce(cellX, cellY, dx, dy);
			}
		}
	}

	int getcolor(int channel) {
		int col=parent.color((channel*37)%255, (channel*91)%255, (channel*211)%255);
		return col;
	}

	private void fluidCanvasStep() {
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

				int collor = getSubPixel(warpX, warpY);

				buffer.set(x, y, collor);
			}
		}
		parent.background(buffer);
	}

	public int getSubPixel(double warpX, double warpY) {
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
			return lerpColor(lerpColor(parent.pixels[indexTopLeft], parent.pixels[indexTopRight], 
					(float) u), lerpColor(parent.pixels[indexBottomLeft], 
							parent.pixels[indexBottomRight], (float) u), (float) v);
		} 
		catch (Exception e) {
			System.out.println("error caught trying to get color for pixel position "
					+ x + ", " + y);
			return bordercolor;
		}
	}

	public int lerpColor(int c1, int c2, float l) {
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

