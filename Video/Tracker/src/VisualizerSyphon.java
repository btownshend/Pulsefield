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
	int gc[]=new int[256];
	
	VisualizerSyphon(PApplet parent, String appName, String serverName) {
		super();
		this.appName=appName;
		this.serverName=serverName;
		for (int i=0;i<256;i++)
			gc[i]=(int)(Math.pow(i/256.0,1.0/2.2)*256+0.5);  // Gamma correction table
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

			if (false) {
				canvas.loadPixels();
				for (int i=0;i<canvas.pixels.length;i++) {
					int pix=canvas.pixels[i];
					if (((pix&0xff0000) != 0xff0000) && ((pix&0xff00)!=0xff00) && ((pix&0xff)!=0xff))
						// Unsaturated pixel -- unmap gamma
						pix=(pix&0xff000000)|(gc[pix>>16 & 0xff]<<16)|(gc[pix>>8 & 0xff]<<8)|(gc[pix & 0xff]);
					canvas.pixels[i]=pix;
					}
				canvas.updatePixels();
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
			float ppm=Math.max(canvas.width/Tracker.getFloorSize().x,canvas.height/Tracker.getFloorSize().y);
			drawImage(g,canvas,Tracker.getFloorCenter().x,Tracker.getFloorCenter().y,
					canvas.width/ppm,canvas.height/ppm);
			wasActive=true;
		}
	}
}
