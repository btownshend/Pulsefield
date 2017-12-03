package com.pulsefield.tracker;
import java.util.HashMap;

import codeanticode.syphon.SyphonClient;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;

public class VisualizerSyphon extends Visualizer {
	SyphonClient client;
	PGraphics canvas=null;
	String appName, serverName;
	boolean wasActive=false;
	boolean captureNextFrame=false;
	float translateX=0f, translateY=0f;
	float rotate=0f;
	float scaleX=1f, scaleY=1f;
	
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
			logger.warning("Unable to connect to syphon server ("+appName+", "+serverName+")");
			logger.warning("Available servers:");
			HashMap<String, String>[] servers = SyphonClient.listServers();
			for (HashMap<String, String> s: servers) {
				logger.warning(s.get("AppName")+": "+s.get("ServerName"));
			}
		} else
			logger.config("Initialized syphon client ("+appName+", "+serverName+")");
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
				logger.warning("Syphon client ("+appName+","+serverName+") not active.");
			super.draw(t, g, p);
			wasActive=false;
		} else if (client.newFrame()) {
			if (!wasActive)
				logger.fine("Syphon client ("+appName+","+serverName+") now active -- new frame.");
			super.draw(t, g, p);
			g.pushMatrix();  // Save the matrix
	    	g.endDraw();     // Calling getGraphics opens a new beginDraw/closeDraw pair which invalidates g's
	    	float oldWidth=(canvas!=null)?canvas.width:0;
	    	float oldHeight=(canvas!=null)?canvas.height:0;
	    	if (!client.active()) {  // Can have a race here -- client may have become inactive while drawing above
	    		logger.fine("Client deactivated");
	    		return;
	    	}
	    	canvas=client.getGraphics(canvas);
	    	if (canvas.format!=PConstants.ARGB) {
	    		logger.info("Changing canvas format from "+canvas.format+" to "+PConstants.ARGB);				
	    		canvas.format=PConstants.ARGB;
	    	}

			if (captureNextFrame) {
				String filename=new String("/tmp/canvas.tif");
				logger.info("Saved frame in "+filename);
				canvas.save(filename);
				captureNextFrame=false;
			}
			if (!wasActive || canvas.width!=oldWidth || canvas.height!=oldHeight)
				logger.info("Syphon canvas size now "+canvas.width+"x"+canvas.height+"; target size="+g.width+"x"+g.height);
			g.beginDraw();  // beginDraw resets the matrix
			g.popMatrix();  // restore it
			initializeContext(t,g);
//			g.scale(-1,1);
			g.imageMode(PConstants.CENTER);
//			if (false) {
//				canvas.loadPixels();
//				for (int x=0;x<3;x++)
//					for (int y=0;y<3;y++) {
//						int c=canvas.pixels[x+canvas.width*y];
//						PApplet.print("("+x+","+y+")="+PApplet.hex(c)+" ");
//					}
//				logger.fine("");
//				//canvas.updatePixels();
//			}
			g.translate(Tracker.getFloorCenter().x,Tracker.getFloorCenter().y);
			g.rotate(rotate);
			g.scale(scaleX, scaleY);
			g.translate(translateX, translateY);
			drawImage(g,canvas,0f,0f,
					Tracker.getFloorSize().x,Tracker.getFloorSize().y);
			wasActive=true;
		}
	}
	
	public void handleMessage(OscMessage msg) {
		logger.fine("Syphon message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");
		
		if (components.length<3 || !components[2].equals("syphon")) 
			logger.warning("Navier: Expected /video/syphon messages, got "+msg.toString());
		else if (components.length==4 && components[3].equals("rotate")) {
			rotate=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("translateX")) {
			translateX=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("translateY")) {
			translateY=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("scaleX")) {
			scaleX=(float) Math.pow(10.0,msg.get(0).floatValue());
		} else if (components.length==4 && components[3].equals("scaleY")) {
			scaleY=(float) Math.pow(10.0,msg.get(0).floatValue());
		} else 
			logger.warning("Unknown Syphon Message: "+msg.toString());
		logger.warning("rotate="+rotate+", translate=("+translateX+","+translateY+"), scale=("+scaleX+","+scaleY+")");
		setTO();
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/syphon/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/video/syphon/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}
	
	public void setTO() {
		setTOValue("scaleX",Math.log10(scaleX),"%.2f");
		setTOValue("scaleY",Math.log10(scaleY),"%.2f");
		setTOValue("rotate",rotate,"%.2f");
		setTOValue("translateX",translateX,"%.4f");
		setTOValue("translateY",translateY,"%.4f");
	}
	
}
