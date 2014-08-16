import processing.core.PApplet;


public class VisualizerCows extends VisualizerIcon {
	final String cowIcons[]={"P1.svg","P2.svg","ToastingCow002.svg"};
	VisualizerCows(PApplet parent) {
		super(parent);
		setIcons(parent,cowIcons);
	}
}
