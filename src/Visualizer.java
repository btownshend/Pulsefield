import processing.core.PApplet;
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
}
