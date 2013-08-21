import java.util.HashMap;

import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

public class VisualizerGrid extends VisualizerPS {
	HashMap<Integer,String> gridValues;
	HashMap<Integer,String> gridColors;
	float gposx[], gposy[];
	float gridwidth, gridheight;
	String song;
	
	VisualizerGrid(PApplet parent) {
		super(parent);
		gridValues = new HashMap<Integer,String>();
		gridColors = new HashMap<Integer,String>();
		gposx=new float[60];
		gposy=new float[60];
		for (int i=0;i<60;i++) {
			int row,col;
			if (i<5) {
				row=0;
				col=i+2;
			} else if (i<12) {
				row=1;
				col=i-5+1;		
			} else if (i<48) {
				row=(i+6)/9;
				col=(i-12)%9;
			} else if (i<55) {
				row=6;
				col=i-48+1;
			} else {
				row=7;
				col=i-55+2;
			}
			//PApplet.println("Grid "+i+" at row="+row+", col="+col);
			gposx[i]=(1+2*col)/18f;
			gposy[i]=1-(1+2*row)/16f;
		}
		gridwidth=1f/9;
		gridheight=1f/8;
	}

	public void draw(PApplet parent, Positions p, PVector wsize) {
		super.draw(parent,p, wsize);
		parent.fill(0);
		parent.stroke(255);
		parent.strokeWeight(1);
		super.drawBorders(parent, true, wsize);

		parent.textSize(16);
		parent.textAlign(PConstants.CENTER,PConstants.CENTER);
		for (int cell: gridValues.keySet()) {
			//PApplet.println("grid "+cell+" = "+gridValues.get(cell)+" "+gridColors.get(cell)+" pos=("+gposx[cell]+","+gposy[cell]+")");
			parent.fill(127,0,0,127);
			parent.strokeWeight(5);
			parent.stroke(127,0,0);
			parent.rect(wsize.x*(gposx[cell]-gridwidth/2),wsize.y*(gposy[cell]-gridheight/2),wsize.x*gridwidth,wsize.y*gridheight);
			parent.fill(255);
			parent.text(gridValues.get(cell),wsize.x*(gposx[cell]-gridwidth/2),wsize.y*(gposy[cell]-gridheight/2),wsize.x*gridwidth,wsize.y*gridheight);
		}
		parent.fill(127);
		parent.textAlign(PConstants.LEFT, PConstants.TOP);
		parent.textSize(24);
		if (song!=null)
			parent.text(song,5,5);
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
			int cell=Integer.parseInt(components[3])-1;  // Cell is 1-60; change to 0
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
		} else if (components[1].equals("grid") && components[2].equals("song")) {
			song=theOscMessage.get(0).stringValue();
			handled=true;
		}
		if (!handled)
			super.handleMessage(theOscMessage);
	}
}

