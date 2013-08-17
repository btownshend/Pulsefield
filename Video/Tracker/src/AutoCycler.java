import java.util.ArrayList;
import java.util.HashMap;

class ModeMap {
	String mode;
	String cmd;
	ModeMap(String mode,String cmd) {
		this.mode=mode;
		this.cmd=cmd;
	}
}

class Setup {
	String ledmode;
	String soundappmode;
	String videomode;
	float weight;

	Setup(String soundappmode, String videomode, String ledmode, float weight) {
		this.ledmode=ledmode;
		this.soundappmode=soundappmode;
		this.videomode=videomode;
		this.weight=weight;
	}
	
	public String toString() {
		return soundappmode+"-"+videomode+"-"+ledmode;
	}
	
}

public class AutoCycler {
	ArrayList<Setup> day,night;
	HashMap<String,String> sound,led,video;

	float daytotal, nighttotal;

	AutoCycler() {
		sound=new HashMap<String,String>();
		led=new HashMap<String,String>();
		video=new HashMap<String,String>();

		video.put("Smoke", "/video/app/buttons/5/1");
		video.put("Navier", "/video/app/buttons/5/2");
		video.put("Tron", "/video/app/buttons/5/3");
		video.put("Ableton", "/video/app/buttons/5/4");
		video.put("DDR", "/video/app/buttons/5/5");
		video.put("Poly", "/video/app/buttons/4/1");
		video.put("Voronoi", "/video/app/buttons/4/2");
		video.put("Guitar", "/video/app/buttons/4/3");
		video.put("Drums", "/video/app/buttons/4/4");

		led.put("Visible", "/led/app/buttons/5/1");
		led.put("FollowPB", "/led/app/buttons/5/2");
		led.put("FollowWht", "/led/app/buttons/5/3");
		led.put("CircSeq", "/led/app/buttons/5/4");
		led.put("Freeze", "/led/app/buttons/5/5");
		led.put("CMerge", "/led/app/buttons/4/1");
		led.put("MtrFollow", "/led/app/buttons/4/2");

		sound.put("Guitar", "/sound/app/buttons/5/1");
		sound.put("CSeq", "/sound/app/buttons/5/2");
		sound.put("HotSpots", "/sound/app/buttons/5/3");
		sound.put("Grid", "/sound/app/buttons/5/4");
		sound.put("Video", "/sound/app/buttons/5/5");

		day=new ArrayList<Setup>();
		night=new ArrayList<Setup>();
		day.add(new Setup("Grid","Smoke","MtrFollow",1f));
		day.add(new Setup("Grid","Smoke","Visible",1f));
		day.add(new Setup("Grid","Smoke","FollowPB",1f));


		night.add(new Setup("Video","Smoke","FollowWht",1f));
		night.add(new Setup("Video","Navier","FollowWht",1f));
		night.add(new Setup("Video","Tron","FollowWht",1f));
		night.add(new Setup("Grid","Ableton","FollowWht",1f));
		night.add(new Setup("Video","DDR","FollowWht",1f));
		night.add(new Setup("Video","Poly","FollowWht",1f));
		night.add(new Setup("Video","Voronoi","FollowWht",2f));
		night.add(new Setup("Video","Guitar","FollowWht",2f));

		daytotal=0;
		for (Setup d: day) {
			daytotal+=d.weight;
		}
		nighttotal=0;
		for (Setup d: night) {
			nighttotal+=d.weight;
		}
	}
	
	/** Change to a new setup
	 * @param daytime - true if running during daytime (no video)
	 */
	public void change(boolean daytime) {
		ArrayList<Setup> setup=daytime?day:night;
		double rpick=Math.random()*(daytime?daytotal:nighttotal);
		for (Setup s:setup) {
			if (s.weight>rpick) {
				System.out.println("Autocycled "+(daytime?"day":"night")+" to "+s.toString()+" (rpick="+rpick+", weight="+s.weight+")");
				Tracker.sendOSC("MPO",video.get(s.videomode),1);
				Tracker.sendOSC("MPO",sound.get(s.soundappmode),1);
				Tracker.sendOSC("MPO",led.get(s.ledmode),1);
				break;
			}
			rpick-=s.weight;	
		}
	}

}
