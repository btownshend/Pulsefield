import java.util.HashMap;

import processing.core.PApplet;
import processing.core.PVector;


public class Positions {
	HashMap<Integer, Position> positions;

	Positions() {
		positions=new HashMap<Integer,Position>();
	}

	public void add(int id, int channel) {
		Position ps=new Position(new PVector(0f,0f),channel, id);
		positions.put(id,ps);
	}


	public void move(int id, int channel, PVector newpos, int groupid, int groupsize, float elapsed) {
		Position ps=positions.get(id);
		if (ps==null) {
			PApplet.println("Unable to locate user "+id+", creating it.");
			add(id,channel);
			ps=positions.get(id);
		}
		ps.move(newpos,groupid, groupsize, elapsed);
		//PApplet.println("ID "+id+" moved to "+newpos);
	}
	public void exit(int id) {
		Position ps=positions.get(id);
		if (ps==null)
			PApplet.println("Unable to locate user "+id+" at exit, ignoring.");
		else
			positions.remove(id);
	}

	public void clear() {
		positions.clear();
	}

	public void setnpeople(int n) {
		if (n!=positions.size()) {
			PApplet.println("Have "+positions.size()+" people, but got message that there are "+n+" .. clearing.");
			positions.clear();
		}
	}
	
	public Position get(int id) {
		return positions.get(id);
	}
}