import delaunay.Pnt;


public class PntWithID extends Pnt {
	int id;
	
	public PntWithID(int id, double... coords) {
		super(coords);
		this.id=id;
	}
	
	public int ID() {
		return id;
	}
}
