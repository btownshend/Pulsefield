package com.pulsefield.tracker;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;

import processing.core.PApplet;

public class VisualizerVDMX extends VisualizerSyphon {
	static final String appName = "VDMX5";
	static final File app = new File("/Applications/VDMX5.app");
	File proj;
	Process pid;
	
	VisualizerVDMX(PApplet parent, String projPath) {
		super(parent,appName,"Main Output");
		proj = new File(projPath);
	}

	@Override
	public void start() {
		super.start();  // Start the syphon client, etc
		if (!app.exists()) {
			logger.warning("VDMX application: "+app.getAbsolutePath()+" does not exist.");
			return;
		}
		if (!proj.exists()) {
			logger.warning("VDMX project: "+proj.getAbsolutePath()+" does not exist.");		
			return;
		}
		String cmd="/usr/bin/open -W -a "+app.getAbsolutePath()+" \""+proj.getAbsolutePath()+"\"";  // Make the open wait for the subprocess so we know when it truly exits
		try {
			// Launch the app
			logger.info("Launching VDMX app with: '"+cmd+"'");
			pid=Runtime.getRuntime().exec(cmd);
			logger.info("pid="+pid.toString()+", isAlive="+pid.isAlive());
		} catch (IOException e) {
		    logger.log(Level.WARNING,"Unable to launch VDMX app: "+app.getAbsolutePath(),e);
		    e.printStackTrace();
		}
	}
	
	@Override
	public void stop() {
		// When this app is deactivated
		
		// Kill the app
		if (pid!=null && pid.isAlive()) {
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
		if (pid==null || !pid.isAlive()) {
			if (pid!=null)
				logger.info("subprocess exitted; status="+pid.exitValue());
			//((Tracker)parent).cycle();
		}
	}

}
