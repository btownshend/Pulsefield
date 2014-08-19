import processing.core.PApplet;
import processing.core.PVector;


public class MusicVisLaser {
	public enum Modes { LINES, POLYGON };
	Modes mode;
	float lval[];
	float ltavg[];
	Fourier fourier;
	
	MusicVisLaser(Fourier f, Modes mode) {
		fourier=f;
		lval=new float[20];
		ltavg=new float[20];
		this.mode=mode;
	}
	
	public void update(PApplet parent) {
		int n=lval.length;
		fourier.calc(n);
		for (int i=0; i<n; i++)  {
			lval[i] = PApplet.lerp(lval[i], (float)Math.pow(fourier.monoFFT[i], 1.0f), .2f);
			ltavg[i] = PApplet.lerp(ltavg[i], (float)Math.pow(fourier.monoFFT[i], 1.0f), .01f);	
		}
	}

	public void drawLaser(PApplet parent, People p) {
		update(parent);
		Laser laser=Laser.getInstance();
		for (Person ps: p.pmap.values()) {  
			laser.cellBegin(ps.id); // Start a cell-specific drawing
			switch (mode) {
			case LINES:
				drawLines(parent,ps.id);
				break;
			case POLYGON:
				drawPolygon(parent,ps.id);
				break;
			default:
				assert(false);
				break;
			}
			laser.cellEnd(ps.id);
		}
	}
	
	private void drawLines(PApplet parent, int id) {
		Laser laser=Laser.getInstance();
		int n=lval.length;
		fourier.calc(n);
		float maxmag=0;
		float scale=2f;
		for (int i=0;i<n;i++) {
			float mag=(float) Math.max(0,lval[i]/ltavg[i]-0.8)/scale;
			mag=Math.min(1.0f, mag);
			if (mag>.05) {
				PVector line=PVector.mult(PVector.fromAngle((float) (i*Math.PI*2/n)),mag);
				laser.line(0,0,line.x,line.y);
			}
			maxmag=Math.max(maxmag,mag);
		}
//		PApplet.println("Max line="+maxmag);
	}
	private void drawPolygon(PApplet parent, int id) {
		Laser laser=Laser.getInstance();
		int n=lval.length;
		float scale=2f;
		for (int i=0;i<n;i++) {
			float mag=(float) Math.max(0,lval[i]/ltavg[i]-0.8)/scale;
			mag=Math.min(1.0f,Math.max(mag, .05f));
			float nextMag=(float) Math.max(0,lval[(i+1)%n]/ltavg[(i+1)%n]-0.8)/scale;
			nextMag=Math.min(1.0f,Math.max(nextMag, .05f));

			PVector pt=PVector.mult(PVector.fromAngle((float) (i*Math.PI*2/n)),mag);
			PVector nextPt=PVector.mult(PVector.fromAngle((float) ((i+1)*Math.PI*2/n)),nextMag);
//			if (PVector.dist(pt,nextPt)>0.02)
				laser.line(pt.x,pt.y,nextPt.x,nextPt.y);
		}
	}
}
