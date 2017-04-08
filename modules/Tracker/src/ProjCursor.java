import processing.core.PVector;

// Cursor for displaying on a projector
public class ProjCursor {
	int proj;
	PVector pos;
	
	public ProjCursor(int proj, float x, float y) {
		this.proj=proj;
		pos=new PVector(x,y);
	}
}
