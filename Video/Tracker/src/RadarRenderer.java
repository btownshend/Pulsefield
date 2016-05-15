

import processing.core.PApplet;
import processing.core.PGraphics;


public class RadarRenderer extends Renderer {
  
  float aura = .25f;
  float orbit = .25f;
  int delay = 2;
  boolean needsClear=true;
  int rotations;

  public RadarRenderer(Fourier f) {
	  super(f,MusicVisLaser.Modes.LINES);
    rotations =  (int) f.fft.getBandWidth();
  }
  
  @Override
  public void start() {
	 needsClear=true;
  }
  
  @Override
  public synchronized void draw(Tracker tracker, PGraphics g)
  {
    g.colorMode(PApplet.RGB, (float)(Math.PI * 2* rotations), 1, 1);
    if (needsClear) {
    	g.background(0);
    	needsClear=false;
    }
    if(fourier.left != null) {
   
      float t = PApplet.map((float)tracker.millis(),0f, delay * 1000f, 0f, (float)Math.PI);   
      int n = fourier.left.length;
      
      // center 
      float w = (float) (g.width/2 + Math.cos(t) * g.width * orbit);
      float h = (float) (g.height/2 + Math.sin(t) * g.height * orbit); 
      
      // size of the aura
      float w2 = g.width * aura, h2 = g.height * aura;
      
      // smoke effect
      if(tracker.frameCount % delay == 0 ) g.image(g.get(),-1.5f,-1.5f,(float)( g.width + 3), (float)(g.height + 3)); 
      
      // draw polar curve 
      float a1=0, x1=0, y1=0, r2=0, a2=0, x2=0, y2=0; 
      for(int i=0; i <= n; i++)
      {
        a1 = a2; x1 = x2; y1 = y2;
        r2 = fourier.left[i % n] ;
        if (fourier.right!=null)
        	r2+=fourier.right[i%n];
        a2 = PApplet.map((float)i,0f, (float)n, 0f, (float)(Math.PI * 2 * rotations));
        x2 = (float) (w + Math.cos(a2) * r2 * w2);
        y2 = (float) (h + Math.sin(a2) * r2 * h2);
        g.stroke(a1, 1, 1, 30);
        // strokeWeight(dist(x1,y1,x2,y2) / 4);
        if(i>0) g.line(x1, y1, x2, y2);
      }
    }
  }
}

