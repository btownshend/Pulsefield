package com.pulsefield.tracker;
import java.util.logging.Logger;

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

/** A musical scale
 * @author Brent Townshend
 *
 */
public class Scale {
	ScaleType stype;
	String key;
	int notes[];  // Midi notes for center octave of scale
    private final static Logger logger = Logger.getLogger(Scale.class.getName());

	/** Create a new scale
	 * @param scaleName name of scale (e.g. "Major")
	 * @param keyname key of scale (e.g. "C#" )
	 */
	public Scale(String scaleName,String keyname) {
		final String keys[]={"C","C#","Db","D","D#","Eb","E","F","F#","Gb","G","G#","Ab","A","A#","Bb","B","B#","Cb"};
		final int keyv[]={60,61,61,62,63,63,64,65,66,66,67,68,68,69,70,70,71,72,72};

		ScaleType scale=ScaleType.find(scaleName);
		if (scale==null) {
			logger.warning("No such scale type: "+scaleName);
			assert(false);
		}
		int[] scpitches=scale.pattern;
		int kpos=-1;
		for (int i=0;i<keys.length;i++)
			if (keys[i].equalsIgnoreCase(keyname))
				kpos=i;
		if (kpos == -1) {
			logger.warning("Unknown key: "+keyname);
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
		}
	}

	/** 
	 * Get number of notes in scale
	 * @return number of notes
	 */
	public int length() {
		return notes.length;
	}
	
	/** 
	 * Get the MIDI pitch for the i-th note in the scale
	 * @param i which note in the scale
	 * @return midi pitch
	 */
	public int get(int i) {
		return notes[i];
	}
	
	
	/** 
	 * Map a value to a MIDI note within scale 
	 * @param val value to map
	 * @param minval minimum of range
	 * @param maxval maximum of range
	 * @param offset increase final pitch by offset steps in scale 
	 * @param noctave number of octaves (pitches cover center-most octaves of scale)
	 * @return MIDI note
	 */
	public int map2note(float val, float minval, float maxval, int offset, int noctave) {
		int notenum=(int)((val-minval)/(maxval-minval)*notes.length*noctave)+offset;
		int pitch=notes[notenum%notes.length];
		pitch=pitch+12*(notenum/12-(int)(noctave/2));
		return pitch;
	}
	
}
