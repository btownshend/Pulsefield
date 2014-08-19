import java.util.HashSet;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PShape;
import processing.core.PVector;

/**
 * Allows for switching between various tracker applications.
 * 
 * @author raphtown
 *
 */
public class VisualizerMenu extends Visualizer {
	int selectingPerson;
	
	/** How many menu items to display at once (includes "next page" item). */
	static final int ITEMS_PER_SCREENFUL = 20;
	
	/** Minimum distance from item to person/other item when initializing (in meters). */
	static final float MIN_DISTANCE = 1.0f;
	
	/** How far a person needs to be to be able to select the menu item (in meters). */
	static final float SELECTION_DISTANCE = 0.3f;
	
	static final float HOTSPOTRADIUS=0.3f;   // Radius of hot spot in meters

	/** Set of items currently being displayed. */
	HashSet<MenuItem> menuItems = new HashSet<MenuItem>();
	PShape cursor;
	static final String CURSOR="handCursor.svg";
	
	VisualizerMenu(PApplet parent) {
		super();
		selectingPerson=-1;
		cursor=parent.loadShape(Tracker.SVGDIRECTORY+CURSOR);
	}
	
	boolean hotSpotCheck(PApplet parent, People people) {
		PVector menuHotSpot = new PVector(2, 1);
		for(Person p : people.pmap.values()) {
			PVector location = p.getOriginInMeters();
			if(PVector.sub(location, menuHotSpot).mag() < HOTSPOTRADIUS) {
				selectingPerson=p.id;
				PApplet.println("Person hit hot spot: "+selectingPerson);
				return true;
			}
		}
		return false;
	}

	/** Next visualizer to serve up.  Call getNextVisualizerIndexSet */
	private int nextVisualizerIndex = 0;
	
	/** Return next visualizer to serve up.  -1 indicates the special 'more' menu category */
	private HashSet<Integer> getNextVisualizerIndexSet() {
		HashSet<Integer> results = new HashSet<Integer>();
		while(true) {
			PApplet.println("getNextSet: "+nextVisualizerIndex+": "+Tracker.selectable[nextVisualizerIndex]);
			if (Tracker.selectable[nextVisualizerIndex])
				results.add(nextVisualizerIndex);
			nextVisualizerIndex++;
			// If we reached the end of the list, we stop adding.
			if (nextVisualizerIndex >= Tracker.visnames.length) {
				nextVisualizerIndex = 0;
				break;
			}
			// Want to leave one slot for more options
			if (results.size() >= ITEMS_PER_SCREENFUL - 1) {
				results.add(-1);
				break;
			}
		}
		PApplet.println("Result="+results);
		return results;
	}
	
	/**
	 * Assign locations to menu items and visualizer to menu items.   We want to position these
	 * items so that they are distanced from people and each other.  Any existing positions are
	 * cleared.
	 * 
	 * @param p Current people on the board.
	 */
	private void initializeItems(People p) {
		menuItems.clear();
		HashSet<PVector> allPositions = new HashSet<PVector>();
		
		for (Person ps: p.pmap.values()) { 
			for (Leg leg : ps.legs) {
				allPositions.add(leg.getOriginInMeters());
			}
		}
		
		HashSet<Integer> visualizerIndices = getNextVisualizerIndexSet();
		for(int index : visualizerIndices) {
			// TODO Improved location finding.
			while(true) {
				PVector proposedPosition = Tracker.normalizedToFloor(new PVector(
						(float)((Math.random() * 1.8) - 0.9), 
						(float)((Math.random() * 1.8) - 0.9)));
				float minDistance = Float.NaN;
				for(PVector otherPosition : allPositions) {
					float currDistance = PVector.dist(proposedPosition, otherPosition);
					if(Float.isNaN(minDistance)) {
						minDistance = currDistance;
					} else if(minDistance > currDistance) {
						minDistance = currDistance;
					}
				}
				// Found a good location
				if(Float.isNaN(minDistance) || minDistance > MIN_DISTANCE) {
					menuItems.add(new MenuItem(proposedPosition, index));
					allPositions.add(proposedPosition);
					break;
				}
			}
		}
	}
	
	@Override
	public void update(PApplet parent, People p) {
		// Initialize menu items if necessary
		if(menuItems.isEmpty()) {
			initializeItems(p);
		}
		
		if (!p.pmap.containsKey(selectingPerson)) {
			// Person has left
			PApplet.println("Selecting person "+selectingPerson+" has left (pmap contains "+p.pmap.size()+" people)");
			((Tracker)parent).setapp(0);
		}
		Person ps=p.pmap.get(selectingPerson);
		if (ps==null) {
			PApplet.println("Selecting person "+selectingPerson+" not found");
			return;
		}
		for(Leg leg : ps.legs) {
			PVector legPosition = leg.getOriginInMeters();
			for(MenuItem item : menuItems) {
				float currDistance = PVector.dist(legPosition, item.position);
				if(currDistance < SELECTION_DISTANCE) {
					if(item.visualizer != -1) {
						((Tracker)parent).setapp(item.visualizer);
					} else {
						initializeItems(p);
					}
				}
			}
		}
	}
	
	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		if (p.pmap.isEmpty())
			return;
		
		parent.ellipseMode(PConstants.CENTER);

		parent.stroke(0xffffffff);
		parent.textAlign(PConstants.CENTER, PConstants.CENTER);
		parent.textSize(25);
		for(MenuItem item : menuItems) {
			PVector pos = Tracker.floorToNormalized(item.position);
			PVector sz = Tracker.mapVelocity(new PVector(2*SELECTION_DISTANCE, 2*SELECTION_DISTANCE));
			parent.fill(0xff000000);
			parent.ellipse((pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2, sz.x*wsize.x/2, sz.y*wsize.y/2);
			parent.fill(0xffffffff);
			parent.text(item.name, (pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2);
		}
		// Draw cursor for selecting person
		Person ps=p.get(selectingPerson);
		float sz=60;
		float scale=Math.min(sz/cursor.width,sz/cursor.height);
		int c=ps.getcolor(parent);
		parent.fill(c,255);
		parent.stroke(c,255);
//		PApplet.println("Drawing cursor with scaling="+scale);
		// cursor seems shifted
		parent.shape(cursor,(ps.getNormalizedPosition().x+1)*wsize.x/2-cursor.width*scale*0.4f, (ps.getNormalizedPosition().y+1)*wsize.y/2-cursor.height*scale*0.2f,cursor.width*scale,cursor.height*scale);
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		Laser laser = Laser.getInstance();
		laser.bgBegin();
		for(MenuItem item : menuItems) {
			laser.shapeBegin(item.name);
			laser.svgfile("app"+item.name+".svg",item.position.x, item.position.y, SELECTION_DISTANCE*1.6f,0.0f);
			laser.circle(item.position.x, item.position.y, SELECTION_DISTANCE);
//			PApplet.println("Circle at "+item.position)
			laser.shapeEnd(item.name);
		}
		laser.bgEnd();
		laser.cellBegin(selectingPerson);
		laser.svgfile(CURSOR,-0.3f,0.4f,1.0f,0.0f);
		laser.cellEnd(selectingPerson);
	}
	
	@Override
	public void start() {
		super.start();
		menuItems.clear();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
	}
}

class MenuItem {
	/** Location of menu item. */
	PVector position;
	
	/** Visualizer index. */
	int visualizer;
	
	/** Name of visualizer. */
	String name;
	
	public MenuItem(PVector position, int visualizer) {
		this.position = position;
		this.visualizer = visualizer;
		 name = visualizer == -1 ? "More" : Tracker.visnames[visualizer];
	}
}
