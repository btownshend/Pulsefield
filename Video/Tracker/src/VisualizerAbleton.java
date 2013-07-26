import java.util.HashMap;

import oscP5.OscMessage;
import processing.core.PApplet;

public class VisualizerAbleton extends VisualizerPS {
	HashMap<Integer,String> gridValues;
	HashMap<Integer,String> gridColors;
	
	VisualizerAbleton(PApplet parent) {
		super(parent);
		gridValues = new HashMap<Integer,String>();
		gridColors = new HashMap<Integer,String>();
	}


	public void draw(PApplet parent, Positions p) {
		super.draw(parent,p);
		for (int cell: gridValues.keySet()) {
			PApplet.println("grid "+cell+" = "+gridValues.get(cell)+" "+gridColors.get(cell));
		}
	}
	
	public void setnpeople(int n) {
		// Ignored for now
	}
	
	public void handleMessage(OscMessage theOscMessage) {
		//PApplet.println("Ableton message: "+theOscMessage.toString());
		boolean handled=false;
		String pattern=theOscMessage.addrPattern();
		String components[]=pattern.split("/");
		
		if (components[1].equals("grid") && components[2].equals("cell")) {
			int cell=Integer.parseInt(components[3]);
			if (components.length == 4) {
				String value=theOscMessage.get(0).stringValue();
				//PApplet.println("cell "+cell+" = "+value);
				if (value.isEmpty()) {
					gridValues.remove(cell);
					gridColors.remove(cell);
				} else
					gridValues.put(cell,value);
				handled=true;
			} else if (components.length ==5 && components[4].equals("color")) {
				String color=theOscMessage.get(0).stringValue();
				gridColors.put(cell,color);
				handled=true;
			}
		} else if (components[1].equals("grid") && components[2].equals("song"))
			handled=true;
		if (!handled)
			PApplet.println("Unhandled Ableton Message: "+pattern);
	}
}

