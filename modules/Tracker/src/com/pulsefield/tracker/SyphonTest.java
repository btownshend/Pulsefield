package com.pulsefield.tracker;
import codeanticode.syphon.SyphonServer;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;

public class SyphonTest {
	static PGraphics c2=null;
	static SyphonServer s2;
	
	static public void draw(PApplet parent) {
		if (c2==null){
			c2 = parent.createGraphics(1000, 720, PConstants.P3D);
			// Create syhpon server to send frames out.
			s2 = new SyphonServer(parent, "Syphon test");
		}
		
		c2.beginDraw();
		c2.background(127);
		c2.lights();
		c2.translate(c2.width/2, c2.height/2);
		c2.rotateX((float)(parent.frameCount * 0.01));
		c2.rotateY((float)(parent.frameCount * 0.01));  
		c2.box(150);
		c2.endDraw();
		//image(c2, 0, 0);
		//beginPGL();
		s2.sendImage(c2);
		//endPGL();
	}

}
