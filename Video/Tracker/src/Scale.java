import processing.core.PApplet;

class ScaleType {
	static final ScaleType scales[]={new ScaleType("Major",new int[]{0, 2, 4, 5, 7, 9, 11})};

	String name;
	int pattern[];
	
	ScaleType(String name, int pattern[]) {
		this.name=name;
		this.pattern=pattern;
	}
	
	static ScaleType find(String name) {
		for (int i=0;i<scales.length;i++)
			if (name.equalsIgnoreCase(scales[i].name))
				return scales[i];
		return null;
	}
}

public class Scale {
	ScaleType stype;
	String key;
	int notes[];  // Midi notes for center octave of scale
	
	Scale(String scaleName,String keyname) {
		final String keys[]={"C","C#","Db","D","D#","Eb","E","F","F#","Gb","G","G#","Ab","A","A#","Bb","B","B#","Cb"};
		final int keyv[]={60,61,61,62,63,63,64,65,66,66,67,68,68,69,70,70,71,72,72};
		ScaleType scale=ScaleType.find(scaleName);

		int[] scpitches=scale.pattern;
		int kpos=-1;
		for (int i=0;i<keys.length;i++)
			if (keys[i].equalsIgnoreCase(keyname))
				kpos=i;
		if (kpos == -1) {
			PApplet.println("Unknown key: "+keyname);
			kpos=1;
		}
		int firstnote=keyv[kpos];
		notes=new int[scpitches.length];
		for (int i=0;i<notes.length;i++)
			notes[i]=firstnote+scpitches[i];

		// Shift to put middle C as close as possible to center
		int offset=(notes[notes.length/2]-60)/12*12;
		for (int i=0;i<notes.length;i++) {
			notes[i]-=offset;
			PApplet.print(" "+notes[i]);
		}
		PApplet.println("");
	}

	public int length() {
		return notes.length;
	}
	
	public int get(int i) {
		return notes[i];
	}
	public int map2note(float val, float minval, float maxval, int offset, int noctave) {
		int notenum=(int)((val-minval)/(maxval-minval)*notes.length*noctave)+offset;
		int pitch=notes[notenum%notes.length];
		pitch=pitch+12*(notenum/12);
		return pitch;
	}
	
}