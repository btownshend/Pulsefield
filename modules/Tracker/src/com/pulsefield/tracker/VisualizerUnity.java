package com.pulsefield.tracker;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;

import processing.core.PApplet;

public class VisualizerUnity extends VisualizerSyphon {
	File app;
	Process pid;
	
	VisualizerUnity(PApplet parent, String appName, String appPath) {
		super(parent,appName,"Main Camera");
		this.app=new File(appPath);
		if (!app.exists()) {
			logger.warning("Unity application: "+app.getAbsolutePath()+" does not exist.");
			assert(false);
		}
	}

	@Override
	public void start() {
		super.start();  // Start the syphon client, etc
		String cmd="open -W "+app.getAbsolutePath();  // Make the open wait for the subprocess so we know when it truly exits
		try {
			// Launch the app
			logger.info("Launching Unity app with: '"+cmd+"'");
			pid=Runtime.getRuntime().exec(cmd);
			logger.info("pid="+pid.toString()+", isAlive="+pid.isAlive());
		} catch (IOException e) {
		    logger.log(Level.WARNING,"Unable to launch Unity app: "+app.getAbsolutePath(),e);
		    e.printStackTrace();
		}
	}
	
	@Override
	public void stop() {
		// When this app is deactivated
		
		// Kill the app
		if (pid.isAlive()) {
			logger.info("Killing process "+app.getAbsolutePath());
			pid.destroyForcibly();
		}
		pid=null;
		super.stop();
	}

	@Override
	public void update(PApplet parent, People p) {
		// Update internal state
		super.update(parent, p);
		if (!pid.isAlive()) {
			logger.info("subprocess exitted; status="+pid.exitValue());
			((Tracker)parent).cycle();
		}
	}

}
