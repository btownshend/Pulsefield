import java.util.HashMap;

import codeanticode.syphon.SyphonClient;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;

public class VisualizerSyphon extends VisualizerDot {
	SyphonClient client;
	PGraphics canvas=null;
	String appName, serverName;
	boolean wasActive=false;
	
	VisualizerSyphon(PApplet parent, String appName, String serverName) {
		super(parent);
		this.appName=appName;
		this.serverName=serverName;
	}

	@Override
	public void start() {
		super.start();

		client=new SyphonClient(Tracker.theTracker,appName,serverName);
		if (client==null) {
			PApplet.println("Unable to connect to syphon server ("+appName+", "+serverName+")");
			PApplet.println("Available servers:");
			HashMap<String, String>[] servers = SyphonClient.listServers();
			for (HashMap<String, String> s: servers) {
				PApplet.println(s.get("AppName")+": "+s.get("ServerName"));
			}
		} else
			PApplet.println("Initialized syphon client ("+appName+", "+serverName+")");
		wasActive=true;
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
		super.update(parent, p);
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
			if (!wasActive || canvas.width!=oldWidth || canvas.height!=oldHeight)
				PApplet.println("Syphon canvas size now "+canvas.width+"x"+canvas.height+"; target size="+g.width+"x"+g.height);
			g.beginDraw();  // beginDraw resets the matrix
			g.popMatrix();  // restore it
			g.scale(-1,1);
			g.imageMode(PConstants.CENTER);
			float ppm=Math.max(canvas.width/Tracker.getFloorSize().x,canvas.height/Tracker.getFloorSize().y);
			g.image(canvas,Tracker.getFloorCenter().x,Tracker.getFloorCenter().y,
					canvas.width/ppm,canvas.height/ppm);
			wasActive=true;
		}
	}
}
