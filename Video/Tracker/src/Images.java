import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import processing.core.PApplet;
import processing.core.PImage;

public class Images {
	List <PImage> imgs;
	Random rgen= new Random();
	
	public Images(String folder) {
		// Retrieve a random image from the given folder within data
		File d=new File(folder);
		if (!d.isDirectory()) {
			PApplet.println(d.getAbsolutePath()+" does not exist or is not a directory.");
			assert(false);
		}
		File allfiles[]=d.listFiles();
		if (allfiles.length<1) {
			PApplet.println("No files found in "+d.getAbsolutePath());
			assert(false);
		}
		imgs=new ArrayList<PImage>();
		for (int i=0;i<allfiles.length;i++) {
			PApplet.println("Loading image from "+allfiles[i].getAbsolutePath());
			PImage img=Tracker.theTracker.loadImage(allfiles[i].getAbsolutePath());
			if (img==null) {
				PApplet.println("Failed load of "+allfiles[i].getAbsolutePath());
			} else {
				imgs.add(img);
			}
		}
		assert(imgs.size()>0);
	}
	
	public PImage getRandom() {
		int choose=rgen.nextInt(imgs.size());
		PApplet.println("getRandom -> "+choose+"/"+imgs.size());
		return imgs.get(choose);
	}
}
