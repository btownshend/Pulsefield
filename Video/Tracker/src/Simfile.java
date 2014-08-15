import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;

import processing.core.PApplet;
import processing.core.PImage;

class NoteData {
	int measure;
	int noteNum;
	int noteLength;
	float timestamp;
	String notes;
	NoteData(int measure, int noteNum, int noteLength, String notes) {
		this.measure=measure;
		this.noteNum=noteNum;
		this.noteLength=noteLength;
		this.notes=notes;
	}
	public String toString() {
		return "["+measure+"."+noteNum+"/"+noteLength+"@"+String.format("%.2f", timestamp)+" "+notes+"]";
	}
}

class Notes  {
	String type,  desc,  difficultyClass;
	int difficultyMeter;
	float radarValues[];
	ArrayList<NoteData> notes;

	Notes(String type, String desc, String difficultyClass, String difficultyMeter, String radarValues, String noteData) {
		this.type=type;
		this.desc=desc;
		this.difficultyClass=difficultyClass;
		this.difficultyMeter=Integer.parseInt(difficultyMeter);
		String rv[]=radarValues.split(",");
		if (rv.length != 5)
			System.err.println("Bad radarValues (expected 5, found "+rv.length+"): "+radarValues);
		this.radarValues = new float[rv.length];
		for (int i=0;i<rv.length;i++)
			this.radarValues[i]=Float.parseFloat(rv[i]);
		// Build notes list
		notes = new ArrayList<NoteData>();
		// Split data by measure
		String measures[] = noteData.split(",");
		int measure=1;
		for (String m: measures) {
			String nsplit[] = m.trim().split("\n");
			int noteNum=1;
			for (String n: nsplit) {
				n=n.trim();
				if (n.startsWith(",")) {
					measure++;
					noteNum=0;
				} else {
					String n2=oneMove(n);
					notes.add(new NoteData(measure,noteNum,nsplit.length,n2));
					System.out.println("Added <"+n+"/"+n2+"> -> "+notes.get(notes.size()-1));
					noteNum++;
				}
			}
			measure++;
		}
	}

	// Reduce a moves string to only one move at a time (no splits/steps)
	public String oneMove(String n) {
		for (int i=0;i<n.length();i++) {
			if (n.charAt(i) != '0') {
				return n.substring(0,i+1)+n.substring(i+1).replaceAll(".", "0");
			}
		}
		return n;
	}
	
	public int measures() {
		return notes.get(notes.size()-1).measure;
	}
	
	// Setup timestamps using given BPMs
	public void setTimestamps(ArrayList<BPM> bpms, float offset) {
		int bpmPtr=0;
		float lastBeat=0f;
		float curTime=offset;
		for (int i=0;i<notes.size();i++) {
			NoteData n=notes.get(i);
			float curBeat=((n.measure-1)+(n.noteNum-1)*1.0f/n.noteLength)*4;
			while (bpmPtr < bpms.size()-1 && bpms.get(bpmPtr+1).fromBeat < lastBeat)
				bpmPtr++;
			curTime+=(curBeat-lastBeat)*60/bpms.get(bpmPtr).bpm;
			n.timestamp=curTime;
			lastBeat=curBeat;
		}
	}
	
	// Get notes starting with measure (1-origin), for nmeasure complete measures
	// Returns all notedata in range, in order
	public ArrayList<NoteData> getNotes(int measure, int nmeasure) {
		ArrayList<NoteData> result = new ArrayList<NoteData>();
		for (NoteData n: notes) {
			if (n.measure>=measure && n.measure<measure+nmeasure)
				result.add(n);
		}
		return result;
	}

	// Get notes in timestamp range
	// Returns all notedata in range, in order
	public ArrayList<NoteData> getNotes(float ts1, float ts2) {
		ArrayList<NoteData> result = new ArrayList<NoteData>();
		for (NoteData n: notes) {
			if (n.timestamp>=ts1 && n.timestamp<=ts2)
				result.add(n);
		}
		return result;
	}

	public String toString() { return toString(false); }
	public String toString(boolean showNotes) {
		StringBuffer results=new StringBuffer("Notes<"+type+","+desc+","+difficultyClass+","+difficultyMeter+",[");
		for (float s: radarValues)
			results.append(s+" ");
		results.append("]");
		results.append(","+notes.size()+"n "+measures()+"m");
		results.append(">");
		if (showNotes)
			for (NoteData n: notes) {
				results.append("\n");
				results.append(n.toString());
			}
		return results.toString();
	}
}
class BPM {
	float fromBeat;
	float bpm;
	BPM(float fromBeat, float bpm) {
		this.fromBeat=fromBeat;
		this.bpm=bpm;
	}
}

public class Simfile {
	File dir;
	HashMap<String,String> tags;
	ArrayList<Notes> notes;
	ArrayList<BPM> bpms;
	
	Simfile() {
		tags=new HashMap<String,String>();
		notes=new ArrayList<Notes>();
		bpms=new ArrayList<BPM>();
	}
	
	// Load from a .SM file
	public final void loadSM(String dir, String filename) throws FileNotFoundException {
		//Note that FileReader is used, not File, since File is not Closeable
		this.dir=new File(dir);
		Scanner scanner = new Scanner(new FileReader(new File(dir,filename)));
		scanner.useDelimiter(";");
		try {
			//first use a Scanner to get each line
			while ( scanner.hasNext() ){
				String token = scanner.next();
				//System.out.println("Got token: <"+token+">");
				token=token.replaceAll("//.*[\r\n]", "");
				String parts[] = token.split(":");
				String label=parts[0].trim();
				if (parts.length == 1 || parts.length == 2) {
					if (label.length()==0)
						continue;
					String value="";
					if (parts.length == 2)
						value=parts[1].trim();
					System.out.println("<"+label+"> = <"+value+">");
					if (label.startsWith("#"))
						tags.put(label.substring(1),value);
					else
						System.err.println("Missing # on tag name: "+token);
				} else if (parts.length==7 && label.equals("#NOTES")) {
					notes.add(new Notes(parts[1].trim(),parts[2].trim(),parts[3].trim(),parts[4].trim(),parts[5].trim(),parts[6].trim()));
				} else {
					System.err.println("Token has unexpected number of elements ("+parts.length+") : "+token);
				}
			}
		}
		finally {
			//ensure the underlying stream is always closed
			//this only has any effect if the item passed to the Scanner
			//constructor implements Closeable (which it does in this case).
			scanner.close();
		}
		preprocess();
	}
	
	void preprocess() {
		// Build BPM array
		bpms.clear();
		String bpmString = getTag("BPMS");
		if (bpmString.isEmpty()) {
			System.err.println("Missing BPM tag, assuming 120bpm");
			bpms.add(new BPM(0f,120f));
		} else {
			for (String s: bpmString.split(",")) {
				String comps[]=s.split("=");
				if (comps.length!=2) {
					System.err.println("Bad BPM string: "+s);
				} else {
					bpms.add(new BPM(Float.parseFloat(comps[0]),Float.parseFloat(comps[1])));
				}
			}
		}
		String offsetTag = getTag("OFFSET");
		float offset=0f;
		if (offsetTag==null)
			System.err.println("No offset tag, assuming offset of zero");
		else
			offset=Float.parseFloat(offsetTag);
		
		// Apply timestamps to notes
		for (Notes n: notes) {
			n.setTimestamps(bpms, offset);
		}
	}

	public PImage getBanner(PApplet parent) {
		String fname=getTag("BANNER");
		PImage result = parent.loadImage(new File(dir,fname).getAbsolutePath());
		return result;
	}
	
	public String toString() {
		StringBuffer result=new StringBuffer();
		for (String tag: tags.keySet()) {
			result.append(tag+": "+tags.get(tag)+"\n");
		}
		for (Notes n: notes) {
			result.append(n.toString(false)+"\n");
		}
		return result.toString();
	}

	public String getTag(String tag) {
		return tags.get(tag);
	}
	
	public int numPatterns() {
		return notes.size();
	}
	
	public int findClosestDifficulty(int difficulty) {
		int best=0;
		for (int i=1;i<notes.size();i++) {
			Notes n=notes.get(i);
			if (Math.abs(n.difficultyMeter-difficulty)< Math.abs(notes.get(best).difficultyMeter-difficulty))
				best=i;
		}
		return best;
	}
	
	public int measures(int pattern) {
		return notes.get(pattern).measures();
	}
	
	public float getduration(int pattern) {
		Notes n=notes.get(pattern);
		return n.notes.get(n.notes.size()-1).timestamp+1.0f;
	}
	public ArrayList<NoteData> getNotes(int pattern, int measure, int nmeasure) {
		return notes.get(pattern).getNotes(measure,nmeasure);
	}

	public ArrayList<NoteData> getNotes(int pattern, float ts1, float ts2) {
		return notes.get(pattern).getNotes(ts1,ts2);
	}

	public static void main(String args[]) {
		try {
			Simfile sf = new Simfile();
			sf.loadSM("/Users/bst/Dropbox/Pulsefield/StepMania/Songs/StepMix 1.0/Impossible Fidelity/","impossible.sm");
			System.out.println(sf.toString());
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			System.err.println("File not found");
		}
	}


}
