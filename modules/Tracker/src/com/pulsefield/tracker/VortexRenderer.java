package com.pulsefield.tracker;


import processing.core.PApplet;
import processing.core.PGraphics;


public class VortexRenderer extends Renderer {

	int n = 48;
	float squeeze = .5f;
	float val[];

	public VortexRenderer(Fourier f) {
		super(f,MusicVisLaser.Modes.LINES); 
		val = new float[n];
	}

	@Override
	public void start() {
	}

	@Override
	public String name() { return "Vortex"; }
	
	@Override
	public synchronized void draw(Tracker tracker, PGraphics g) {
		if(fourier.left != null) {  

			float t = PApplet.map((float)tracker.millis(),0f, 3000f, 0f, (float)(2*Math.PI));
			float dx = g.width / n;
			float dy = (float)(g.height / n * .5);
			fourier.calc(n);

			g.colorMode(PApplet.HSB, n, n, n);
			g.rectMode(PApplet.CORNERS);
			g.noStroke();
			
			// rotate slowly
			g.background(0); g.lights();
			g.translate(g.width/2, g.height, -g.width/2);
			g.rotateZ(PApplet.HALF_PI); 
			g.rotateY((float)(-2.2 - PApplet.HALF_PI + ((float)tracker.mouseY)/g.height * PApplet.HALF_PI));
			g.rotateX(t);
			g.translate(0,g.width/4,0);
			g.rotateX(t);

			// draw coloured slices
			for(int i=0; i < n; i++)
			{
				val[i] = PApplet.lerp(val[i], (float)Math.pow(fourier.monoFFT[i] * (i+1), squeeze), .1f);
				float x = PApplet.map(i, 0, n, g.height, 0);
				float y = PApplet.map(val[i], 0, fourier.maxFFT, 0, g.width/2);
				g.pushMatrix();
				g.translate(x, 0, 0);
				g.rotateX((float)(Math.PI/16 * i));
				g.fill(i, (float)(n * .7 + i * .3), n-i);
				g.box(dy, dx + y, dx + y);
				g.popMatrix();
			}
		}
	}
}


