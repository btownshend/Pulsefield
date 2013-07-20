import java.util.HashMap;
import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;

class GridData {
	int id;
	int channel;
	int prevgrid;
	boolean current;
	GridData() { id=-1; prevgrid=0; }
}

public class VisualizerTron extends Visualizer {
	final int gridWidth = 30;
	final int gridHeight = 30;
	HashMap<Integer, Position> systems;
	int width, height;
	GridData grid[];

	VisualizerTron(PApplet parent) {
		super();
		systems = new HashMap<Integer,Position>();
		grid=new GridData[gridWidth*gridHeight];
		for (int i=0;i<grid.length;i++)
			grid[i]=new GridData();
		width=parent.width;
		height=parent.height;
	}

	public void add(int id, int channel) {
		Position ps=new Position(new PVector(0f,0f),channel);
		systems.put(id,ps);
	}

	int postogrid(PVector p) {
		int gpos=postogridx(p.x)*gridHeight+postogridy(p.y);
		return gpos;
	}

	int postogridx(float x) {
		int gx=(int)(x*gridWidth/width);
		if (gx<0) gx=0;
		if (gx>=gridWidth) gx=gridWidth-1;
		return gx;
	}
	int postogridy(float y) {
		int gy=(int)(y*gridHeight/height);
		if (gy<0) gy=0;
		if (gy>=gridHeight) gy=gridHeight-1;
		return gy;

	}

	String gdisp(int grid) {
		int gx=grid/gridHeight;
		int gy=grid-gx*gridHeight;
		return "("+gx+","+gy+")";
	}
	void clear(int id) {
		for (int i=0;i<grid.length;i++)
			if (grid[i].id==id || id==-1)
				grid[i].id=-1;	
	}

	int findstep(int oldgpos,int gpos) {
		int dx=(int)(gpos/gridHeight)-(int)(oldgpos/gridHeight);
		int dy=gpos-oldgpos-dx*gridHeight;
		assert(dx!=0 || dy!=0);
		if (Math.abs(dx)>Math.abs(dy))
			return oldgpos+((dx>0)?gridHeight:-gridHeight);
		else
			return oldgpos+((dy>0)?1:-1);
	}

	public void move(int id, int channel, PVector newpos, float elapsed) {
		Position ps=systems.get(id);
		if (ps==null) {
			PApplet.println("Unable to locate user "+id+", creating it.");
			add(id,channel);
			ps=systems.get(id);
		}
		if (ps.enabled) {
			int oldgpos=postogrid(ps.origin);
			int priorgpos=grid[oldgpos].prevgrid;
			int gpos=postogrid(newpos);
			if (gpos==priorgpos) {
				// Retracing prior step, undo last step
				grid[oldgpos].id=-1;
				grid[gpos].current=true;
			} else if (gpos==grid[priorgpos].prevgrid) {
				// Back up 2 steps, probably a diagonal
				grid[oldgpos].id=-1;
				grid[priorgpos].id=-1;
				grid[gpos].current=true;
			} else
				while (oldgpos!=gpos) {
					// Move 1 grid step at a time, always manhattan type move (no diagonals)
					int stepgpos=findstep(oldgpos,gpos);
					PApplet.println("findstep("+oldgpos+","+gpos+") -> "+stepgpos);
					GridData g=grid[stepgpos];
					if (g.id==-1) {
						PApplet.println("Set grid "+gdisp(stepgpos)+" to "+id+" prev="+gdisp(oldgpos));
						grid[stepgpos].id=id;
						grid[stepgpos].prevgrid=oldgpos;
						grid[stepgpos].channel=channel;
						grid[stepgpos].current=true;
						grid[oldgpos].current=false;
					} else {
						// Collision, clear out this ID
						PApplet.println("Collision of ID "+id+" with "+g.id+" at pos="+gdisp(stepgpos)+", prev="+gdisp(g.prevgrid));						
						clear(id);
						ps.enable(false);
						break;
					}
					oldgpos=stepgpos;
				}
		}
		ps.move(newpos,elapsed);
		if (!ps.enabled) {
			PApplet.println("Enabling ID "+id);
			ps.enable(true);
		}
	}

	int getcolor(PApplet parent, int channel) {
		int col=parent.color((channel*37+99)%255, (channel*91)%255, (channel*211)%255);
		return col;
	}

	public void draw(PApplet parent) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		for (int i=0;i<gridWidth;i++)
			for (int j=0;j<gridHeight;j++) {
				GridData g=grid[i*gridHeight+j];
				int gid=g.id;
				if (gid!=-1) {
					if (g.current) {
						parent.fill(parent.color(255,0,0));
						parent.rect(i*parent.width/gridWidth, j*parent.height/gridHeight, parent.width/gridWidth, parent.height/gridHeight);
					}
					parent.fill(getcolor(parent,g.channel));
					parent.rect(i*parent.width/gridWidth+1, j*parent.height/gridHeight+1, parent.width/gridWidth-2, parent.height/gridHeight-2);

				}
			}	
		if (systems.isEmpty()) {
			parent.fill(50, 255, 255);
			parent.textAlign(PConstants.CENTER);
			parent.textSize(32);
			parent.text("Waiting for users...", parent.width/2, parent.height/2);
		}
	}

	public void exit(int id) {
		if (systems.containsKey(id)) {
			systems.get(id).enable(false);
			clear(id);
		} 
		else
			PApplet.println("Unable to locate id "+id);
	}

	public void clear() {
		systems.clear();
		clear(-1);
	}
}

