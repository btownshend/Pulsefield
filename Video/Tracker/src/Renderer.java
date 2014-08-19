import processing.core.PApplet;
import processing.core.PVector;


public abstract class Renderer {
	Fourier fourier;
	float lval[];
	float ltavg[];
	
	Renderer(Fourier f) {
		fourier=f;
		lval=new float[20];
		ltavg=new float[20];
	}
	public void start() {}
	public void stop() {}
	public abstract void draw(PApplet parent); 
	public void update(PApplet parent) {
		int n=lval.length;
		fourier.calc(n);
		for (int i=0; i<n; i++)  {
			lval[i] = PApplet.lerp(lval[i], (float)Math.pow(fourier.monoFFT[i], 1.0f), .2f);
			ltavg[i] = PApplet.lerp(ltavg[i], (float)Math.pow(fourier.monoFFT[i], 1.0f), .01f);	
		}
	}
	public abstract void drawLaserPerson(PApplet parent, int id);
	

	public void drawLaserPerson1(PApplet parent, int id) {
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
	public void drawLaserPerson2(PApplet parent, int id) {
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
