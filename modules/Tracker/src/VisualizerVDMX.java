import java.io.File;
import java.io.IOException;

import processing.core.PApplet;

public class VisualizerVDMX extends VisualizerSyphon {
	static final String appName = "VDMX5";
	static final File app = new File("/Applications/VDMX5.app");
	File proj;
	Process pid;
	
	VisualizerVDMX(PApplet parent, String projPath) {
		super(parent,appName,"VideoOut");
		proj = new File(projPath);
	}

	@Override
	public void start() {
		super.start();  // Start the syphon client, etc
		if (!app.exists()) {
			PApplet.println("VDMX application: "+app.getAbsolutePath()+" does not exist.");
			return;
		}
		if (!proj.exists()) {
			PApplet.println("VDMX project: "+proj.getAbsolutePath()+" does not exist.");		
			return;
		}
		String cmd="open -W -a "+app.getAbsolutePath()+" \""+proj.getAbsolutePath()+"\"";  // Make the open wait for the subprocess so we know when it truly exits
		try {
			// Launch the app
			PApplet.println("Launching VDMX app with: '"+cmd+"'");
			pid=Runtime.getRuntime().exec(cmd);
			PApplet.println("pid="+pid.toString()+", isAlive="+pid.isAlive());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			PApplet.println("Unable to launch VDMX app: "+app.getAbsolutePath());
			e.printStackTrace();
		}
	}
	
	@Override
	public void stop() {
		// When this app is deactivated
		
		// Kill the app
		if (pid!=null && pid.isAlive()) {
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
		if (pid==null || !pid.isAlive()) {
			if (pid!=null)
				PApplet.println("subprocess exitted; status="+pid.exitValue());
			((Tracker)parent).cycle();
		}
	}

}
