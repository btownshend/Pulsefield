import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

public abstract class Visualizer {
	Visualizer() {
	}

	abstract public void draw(PApplet parent);

	public void update(PApplet parent) { }

	abstract public void add(int id, int channel);
	abstract public void move(int id, int channel, PVector newpos, float elapsed);

	abstract public void exit(int id);
	abstract public void clear();

	public void stats() { }

	public void drawBorders(PApplet parent, boolean octagon) {
		if (octagon) {
			parent.beginShape();
			float gapAngle=(float)(10f*Math.PI /180);
			for (float angle=gapAngle/2;angle<2*Math.PI;angle+=(2*Math.PI-gapAngle)/8)
				parent.vertex((float)((Math.sin(angle)+1)*parent.width/2),(float)((Math.cos(angle)+1)*parent.height/2));
			parent.endShape(PConstants.OPEN);
		} else {
			parent.line(0, 0, parent.width-1, 0);
			parent.line(0, 0, 0, parent.height-1);
			parent.line(parent.width-1, 0, parent.width-1, parent.height-1);
			parent.line(0, parent.height-1, parent.width-1, parent.height-1);
		}
	}


}
