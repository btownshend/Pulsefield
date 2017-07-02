package com.pulsefield.tracker;


import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;


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
  public String name() { return "Radar"; }

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
      PVector center=Tracker.getFloorCenter();
      PVector sz=Tracker.getFloorSize();
      float w = (float) (center.x + Math.cos(t) * sz.x * orbit);
      float h = (float) (center.y + Math.sin(t) * sz.y * orbit); 
      // size of the aura
      float w2 = sz.x * aura, h2 = sz.y * aura;
      
      float signalLevel=Math.abs(fourier.left[20]);
      //logger.fine("signal="+signalLevel);
      // smoke effect
      if(tracker.frameCount % delay == 0 ) {
    	  PVector smokeShift=new PVector(tracker.random(-signalLevel,signalLevel),tracker.random(-signalLevel,signalLevel));
    	  //PVector smokeShift=new PVector(0.005f,0.005f);
    	  g.imageMode(PConstants.CORNER);
    	  PImage frame=g.get();
    	  g.tint(255,200);
    	  g.image(frame,center.x-sz.x/2+smokeShift.x,center.y-sz.y/2+smokeShift.y,sz.x+smokeShift.x, sz.y+smokeShift.y); 
    	  g.tint(255,255);
      }
      
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

