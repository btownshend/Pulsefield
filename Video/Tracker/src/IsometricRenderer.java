

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;


// the code for the isometric renderer was deliberately taken from Jared C.'s wavy sketch 
// ( http://www.openprocessing.org/visuals/?visualID=5671 )

public class IsometricRenderer extends Renderer {

  int r = 9;
  float squeeze = .5f;

  float a, d;
  float val[];
  int n;
  PGraphics pg;
  
  public IsometricRenderer(PApplet parent, Fourier f) {
    super(f);
    n = (int)Math.ceil(Math.sqrt(2) * r);
    d = Math.min(parent.width, parent.height) / r / 5;
    val = new float[n];
    // Offscreen P2D renderer (fastest)
	PApplet.println("Creating render with size "+parent.width+"x"+parent.height);
    pg = parent.createGraphics(parent.width, parent.height,PConstants.P2D);
    pg.noSmooth();
//    
    // Alternatively use Java2D (sharper)
    // pg = createGraphics(width, height, JAVA2D);
 }

  @Override
  public void start() { 


  }

  @Override
  public void stop() {
  }
  
  @Override
  public void draw(PApplet parent) {
    if (fourier.left != null) {
  	  fourier.calc(n);;

  	  // actual values react with a delay
  	  for (int i=0; i<n; i++) val[i] = PApplet.lerp(val[i], (float)Math.pow(fourier.monoFFT[i], squeeze), .1f);
  	  
      pg.beginDraw();
      pg.colorMode(PApplet.HSB, 100,100,100); 
      pg.stroke(0);
      pg.lights();
 
      a -= 0.08; 
      pg.background(0);  
      for (int x = -r; x <= r; x++) { 
        for (int z = -r; z <= r; z++) { 
          int y = (int) ( parent.height/3 * val[(int) PApplet.dist(x, z, 0, 0)]); 

          float xm = x*d - d/2; 
          float xt = x*d + d/2; 
          float zm = z*d - d/2; 
          float zt = z*d + d/2; 

          int w0 = (int) parent.width/2; 
          int h0 = (int) parent.height * 2/3; 

          int isox1 = (int)(xm - zm + w0); 
          int isoy1 = (int)((xm + zm) * 0.5 + h0); 
          int isox2 = (int)(xm - zt + w0); 
          int isoy2 = (int)((xm + zt) * 0.5 + h0); 
          int isox3 = (int)(xt - zt + w0); 
          int isoy3 = (int)((xt + zt) * 0.5 + h0); 
          int isox4 = (int)(xt - zm + w0); 
          int isoy4 = (int)((xt + zm) * 0.5 + h0); 

          // pg.hint(DISABLE_DEPTH_TEST);
          int color=(int)(y*10.0/d);
//          PApplet.println("color="+color);
          if (color>100)
        	  color=100;
          pg.fill (color,25,100); 
          pg.quad(isox2, isoy2-y, isox3, isoy3-y, isox3, isoy3+d, isox2, isoy2+d); 
          pg.fill (color,50,100); 
          pg.quad(isox3, isoy3-y, isox4, isoy4-y, isox4, isoy4+d, isox3, isoy3+d); 
          pg.fill(color,100,100);
          pg.quad(isox1, isoy1-y, isox2, isoy2-y, isox3, isoy3-y, isox4, isoy4-y); 
          // pg.hint(ENABLE_DEPTH_TEST);
        }
      }
      pg.endDraw();
      parent.image(pg, 0, 0);
    }
  }
  
  @Override
  public void drawLaserPerson(PApplet parent, int id) {
	  drawLaserPerson2(parent,id);
  }
}

