import java.io.File;
import java.io.IOException;

import processing.core.PApplet;

public class VisualizerUnity extends VisualizerSyphon {
	File app;
	Process pid;
	
	VisualizerUnity(PApplet parent, String appName, String appPath) {
		super(parent,appName,"Main Camera");
		this.app=new File(appPath);
		if (!app.exists()) {
			PApplet.println("Unity application: "+app.getAbsolutePath()+" does not exist.");
			assert(false);
		}
	}

	@Override
	public void start() {
		super.start();  // Start the syphon client, etc
		String cmd="open -W "+app.getAbsolutePath();  // Make the open wait for the subprocess so we know when it truly exits
		try {
			// Launch the app
			PApplet.println("Launching Unity app with: '"+cmd+"'");
			pid=Runtime.getRuntime().exec(cmd);
			PApplet.println("pid="+pid.toString()+", isAlive="+pid.isAlive());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			PApplet.println("Unable to launch Unity app: "+app.getAbsolutePath());
			e.printStackTrace();
		}
	}
	
	@Override
	public void stop() {
		// When this app is deactivated
		
		// Kill the app
		if (pid.isAlive()) {
			PApplet.println("Killing process "+app.getAbsolutePath());
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
			PApplet.println("subprocess exitted; status="+pid.exitValue());
			((Tracker)parent).cycle();
		}
	}

}
