package com.pulsefield.tracker;

import java.io.File;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import processing.core.PApplet;

import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.Clip;



/* Playsound : A simple class which lazy-loads a sound file on first playback
 * and plays it on command.  Specify filenames for playback as e.g. ./data/life/spaceshipEb.au.
 * 
 * This class is intended to (BUT DOESN'T YET) have two operation modes; one that plays locally and 
 * another that routes the playback through ableton.
 * 
 * TODO: Make this a singleton? 
 */
public final class PlaySound {	
	// Audio Clip map.
	HashMap<String, Clip> acMap = new HashMap<String, Clip>();

	public void play(String soundfilename) {
		// Check map for existing clip and load if necessary.
		if (!acMap.containsKey(soundfilename)) {
			Clip clip;

			try {
				File soundfile = new File(soundfilename);
				AudioInputStream sound = AudioSystem.getAudioInputStream(soundfile);

				clip = AudioSystem.getClip();
				clip.open(sound);
				acMap.put(soundfilename, clip);
			} catch (Exception e) {
				PApplet.println("failed to load sound '" + soundfilename + "' due to exception : " + e.getMessage());
				return;
			}
		}

		try
		{
			acMap.get(soundfilename).setFramePosition(0);
			acMap.get(soundfilename).start();
		}
		catch (Exception e)
		{
			PApplet.println("failed to play sound '" + soundfilename + "' due to exception : " + e.getMessage());
		}
	}

	public void stopAll() {
		Iterator<Entry<String, Clip>> it = acMap.entrySet().iterator();
		while (it.hasNext()) {
			Map.Entry<String, Clip> pair = it.next();
			pair.getValue().stop();
		}
	}
}