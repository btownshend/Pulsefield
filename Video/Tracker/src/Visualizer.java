import processing.core.PApplet;
import processing.core.PConstants;

public abstract class Visualizer {
	Visualizer() {
	}

	public void draw(PApplet parent, Positions p) {
		if (p.positions.isEmpty()) {
			parent.fill(50, 255, 255);
			parent.textAlign(PConstants.CENTER);
			parent.textSize(32);
			parent.text("Waiting for users...", parent.width/2, parent.height/2);
		}
	}

	abstract public void update(PApplet parent, Positions p);

	public void stats() { }

	public void drawBorders(PApplet parent, boolean octagon) {
		if (octagon) {
			parent.beginShape();
			float gapAngle=(float)(10f*Math.PI /180);
			for (float angle=gapAngle/2;angle<2*Math.PI;angle+=(2*Math.PI-gapAngle)/8)
				parent.vertex((float)((Math.sin(angle+Math.PI)+1)*parent.width/2),(float)((Math.cos(angle+Math.PI)+1)*parent.height/2));
			parent.endShape(PConstants.OPEN);
		} else {
			parent.line(0, 0, parent.width-1, 0);
			parent.line(0, 0, 0, parent.height-1);
			parent.line(parent.width-1, 0, parent.width-1, parent.height-1);
			parent.line(0, parent.height-1, parent.width-1, parent.height-1);
		}
	}

}
