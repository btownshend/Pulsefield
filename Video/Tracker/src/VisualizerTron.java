import java.util.HashMap;
import java.util.Iterator;

import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;

class GridData {
	int id;
	int channel;
	int prevgrid;
	int exploding;
	GridData() { id=-1; prevgrid=-1; exploding=-1; }
	void set(int id, int prevgrid, int channel) {
		assert(this.id==-1 || this.exploding>0);
		this.id=id;
		this.channel=channel;
		this.prevgrid=prevgrid;
		this.exploding=-1;
	}
}

public class VisualizerTron extends Visualizer {
	final int gridWidth = 20;
	final int gridHeight = 20;
	int width, height;
	GridData grid[];
	HashMap<Integer,Integer> currentgrid;

	VisualizerTron(PApplet parent) {
		super();
		grid=new GridData[gridWidth*gridHeight];
		for (int i=0;i<grid.length;i++)
			grid[i]=new GridData();
		width=parent.width;
		height=parent.height;
		currentgrid=new HashMap<Integer,Integer>();
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
			if (grid[i].id==id || id==-1  && grid[i].exploding==-1) {
				grid[i].exploding=1;
			}
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

	public void update(PApplet parent, Positions positions) {
		for (int id: positions.positions.keySet()) {
			Position ps=positions.get(id);
			PVector newpos=ps.origin;
			int gpos=postogrid(newpos);
			if (currentgrid.containsKey(id)) {
				int oldgpos=currentgrid.get(id);
				assert(grid[oldgpos].id == id);
				int priorgpos=grid[oldgpos].prevgrid;
				if (gpos==priorgpos && grid[oldgpos].id==id) {
					// Retracing prior step, undo last step
					grid[oldgpos].id=-1;
					currentgrid.put(id, gpos);
				} else if (priorgpos!=-1 && gpos==grid[priorgpos].prevgrid) {
					// Back up 2 steps, probably a diagonal
					grid[oldgpos].id=-1;
					grid[priorgpos].id=-1;
					currentgrid.put(id, gpos);
				} else
					while (oldgpos!=gpos) {
						// Move 1 grid step at a time, always manhattan type move (no diagonals)
						int stepgpos=findstep(oldgpos,gpos);
						//PApplet.println("findstep("+oldgpos+","+gpos+") -> "+stepgpos);
						GridData g=grid[stepgpos];
						if (g.id==-1 || g.exploding>0) {
							//PApplet.println("Set grid "+gdisp(stepgpos)+" to "+id+" prev="+gdisp(oldgpos));
							grid[stepgpos].set(id,oldgpos,ps.channel);
							currentgrid.put(id, stepgpos);
						} else {
							// Collision, clear out this ID
							PApplet.println("Collision of ID "+id+" with "+g.id+" at pos="+gdisp(stepgpos)+", prev="+gdisp(g.prevgrid));						
							clear(id);
							currentgrid.remove(id);
							break;
						}
						oldgpos=stepgpos;
					}
			} else if (grid[gpos].id==-1 || grid[gpos].exploding>0) {
				// Not on map, just put us there
				grid[gpos].set(id,-1,ps.channel);
				currentgrid.put(id, gpos);
			} else {
				// On top of someone
				//PApplet.println("ID "+id+" is on top of "+grid[gpos].id);
			}
		}
		for (Iterator<Integer> iter = currentgrid.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!positions.positions.containsKey(id)) {
				clear(id);
				iter.remove();
			}
		}
	}

	int getcolor(PApplet parent, int channel) {
		int col=parent.color((channel*37+99)%255, (channel*91)%255, (channel*211)%255);
		return col;
	}

	public void draw(PApplet parent, Positions p) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);
		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0,32,0);
		drawBorders(parent,true);
		parent.stroke(0);
		for (int i=0;i<gridWidth;i++)
			for (int j=0;j<gridHeight;j++) {
				GridData g=grid[i*gridHeight+j];
				int gid=g.id;
				if (gid!=-1) {
					if (g.exploding>0) {
						final int explosionFrames = 400;
						parent.fill(getcolor(parent,g.channel));
						int w = parent.width*(explosionFrames-g.exploding)/explosionFrames/gridWidth;
						int h =parent.height*(explosionFrames-g.exploding)/explosionFrames/gridHeight;
						parent.rect(i*parent.width/gridWidth, j*parent.height/gridHeight+g.exploding,w,h);
						parent.rect(i*parent.width/gridWidth, j*parent.height/gridHeight-g.exploding, w,h);
						parent.rect(i*parent.width/gridWidth+g.exploding, j*parent.height/gridHeight, w,h);
						parent.rect(i*parent.width/gridWidth-g.exploding, j*parent.height/gridHeight, w,h);
						if (g.exploding<explosionFrames)
							grid[i*gridHeight+j].exploding+=10;
						else {
							grid[i*gridHeight+j].exploding=-1;
							grid[i*gridHeight+j].id=-1;
						}
					} else {
						int inset=1;
						if (currentgrid.containsKey(gid) && currentgrid.get(gid)==i*gridHeight+j) {
							parent.fill(parent.color(255,0,0));
							parent.rect(i*parent.width/gridWidth, j*parent.height/gridHeight, parent.width/gridWidth, parent.height/gridHeight);
							inset=2;
						}
						parent.fill(getcolor(parent,g.channel));
						parent.rect(i*parent.width/gridWidth+inset, j*parent.height/gridHeight+inset, parent.width/gridWidth-2*inset, parent.height/gridHeight-2*inset);
					}
				}
			}	
		super.draw(parent, p);
	}
}

