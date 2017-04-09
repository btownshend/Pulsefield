package com.pulsefield.tracker;
import java.util.HashMap;

import codeanticode.syphon.SyphonClient;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.opengl.PGL;

public class VisualizerSyphon extends Visualizer {
	SyphonClient client;
	PGraphics canvas=null;
	String appName, serverName;
	boolean wasActive=false;
	boolean captureNextFrame=false;
	
	VisualizerSyphon(PApplet parent, String appName, String serverName) {
		super();
		this.appName=appName;
		this.serverName=serverName;
	}

	@Override
	public void start() {
		super.start();
		client=initClient(appName,serverName);
		wasActive=true;
	}
	
	public SyphonClient initClient(String appName,String serverName) {
		SyphonClient client=new SyphonClient(Tracker.theTracker,appName,serverName);
		if (client==null) {
			PApplet.println("Unable to connect to syphon server ("+appName+", "+serverName+")");
			PApplet.println("Available servers:");
			HashMap<String, String>[] servers = SyphonClient.listServers();
			for (HashMap<String, String> s: servers) {
				PApplet.println(s.get("AppName")+": "+s.get("ServerName"));
			}
		} else
			PApplet.println("Initialized syphon client ("+appName+", "+serverName+")");
		return client;
	}
	
	@Override
	public void stop() {
		client.stop();
		client=null;
		super.stop();
		// When this app is deactivated
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		assert (client!=null);
		if (!client.active()) {
			if (wasActive)
				PApplet.println("Syphon client ("+appName+","+serverName+") not active.");
			super.draw(t, g, p);
			wasActive=false;
		} else if (client.newFrame()) {
			if (!wasActive)
				PApplet.println("Syphon client ("+appName+","+serverName+") now active -- new frame.");
			super.draw(t, g, p);
			g.pushMatrix();  // Save the matrix
	    	g.endDraw();     // Calling getGraphics opens a new beginDraw/closeDraw pair which invalidates g's
	    	float oldWidth=(canvas!=null)?canvas.width:0;
	    	float oldHeight=(canvas!=null)?canvas.height:0;
	    	if (!client.active()) {  // Can have a race here -- client may have become inactive while drawing above
	    		PApplet.println("Client deactivated");
	    		return;
	    	}
	    	canvas=client.getGraphics(canvas);
	    	if (canvas.format!=PConstants.ARGB) {
	    		PApplet.println("Changing canvas format from "+canvas.format+" to "+PConstants.ARGB);				
	    		canvas.format=PConstants.ARGB;
	    	}

			if (captureNextFrame) {
				String filename=new String("/tmp/canvas.tif");
				PApplet.println("Saved frame in "+filename);
				canvas.save(filename);
				captureNextFrame=false;
			}
			if (!wasActive || canvas.width!=oldWidth || canvas.height!=oldHeight)
				PApplet.println("Syphon canvas size now "+canvas.width+"x"+canvas.height+"; target size="+g.width+"x"+g.height);
			g.beginDraw();  // beginDraw resets the matrix
			g.popMatrix();  // restore it
			initializeContext(t,g);
//			g.scale(-1,1);
			g.imageMode(PConstants.CENTER);
			if (false) {
				canvas.loadPixels();
				for (int x=0;x<3;x++)
					for (int y=0;y<3;y++) {
						int c=canvas.pixels[x+canvas.width*y];
						PApplet.print("("+x+","+y+")="+PApplet.hex(c)+" ");
					}
				PApplet.println("");
				//canvas.updatePixels();
			}
			drawImage(g,canvas,Tracker.getFloorCenter().x,Tracker.getFloorCenter().y,
					Tracker.getFloorSize().x,Tracker.getFloorSize().y);
			wasActive=true;
		}
	}
}
