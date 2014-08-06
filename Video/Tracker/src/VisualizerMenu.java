import java.util.HashSet;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;

/**
 * Allows for switching between various tracker applications.
 * 
 * @author raphtown
 *
 */
public class VisualizerMenu extends VisualizerDot {
	
	/** How many menu items to display at once (includes "next page" item). */
	static final int ITEMS_PER_SCREENFUL = 5;
	
	/** Minimum distance from item to person/other item when initializing (in meters). */
	static final float MIN_DISTANCE = 1.0f;
	
	/** How far a person needs to be to be able to select the menu item (in meters). */
	static final float SELECTION_DISTANCE = 0.5f;
	
	/** Set of items currently being displayed. */
	HashSet<MenuItem> menuItems = new HashSet<MenuItem>();
	
	
	VisualizerMenu(PApplet parent) {
		super(parent);
	}
	
	/** Next visualizer to serve up.  Call getNextVisualizerIndexSet */
	private int nextVisualizerIndex = 0;
	
	/** Return next visualizer to serve up.  -1 indicates the special 'more' menu category */
	private HashSet<Integer> getNextVisualizerIndexSet() {
		HashSet<Integer> results = new HashSet<Integer>();
		while(true) {
			results.add(nextVisualizerIndex++);
			// If we reached the end of the list, we stop adding.
			if (nextVisualizerIndex >= Tracker.visnames.length) {
				nextVisualizerIndex = 0;
				results.add(-1);
				break;
			}
			// Want to leave one slot for more options
			if (results.size() >= ITEMS_PER_SCREENFUL - 1) {
				results.add(-1);
				break;
			}
		}
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
				PVector proposedPosition = Tracker.unMapPosition(new PVector(
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
		
		person:
		for(Person ps: p.pmap.values()) { 
			for(Leg leg : ps.legs) {
				PVector legPosition = leg.getOriginInMeters();
				for(MenuItem item : menuItems) {
					float currDistance = PVector.dist(legPosition, item.position);
					if(currDistance < SELECTION_DISTANCE + leg.getDiameterInMeters() / 2) {
						if(item.visualizer != -1) {
							((Tracker)parent).setapp(item.visualizer);
						} else {
							initializeItems(p);
						}
						break person;
					}
				}
			}
		}
	}
	
	@Override
	public void draw(PApplet parent, People p, PVector wsize) {
		super.draw(parent, p, wsize);
		parent.ellipseMode(PConstants.CENTER);
		parent.fill(0xffffffff,255);
		parent.stroke(0xffffffff,255);
		parent.textAlign(PConstants.CENTER, PConstants.CENTER);
		parent.textSize(50);
		for(MenuItem item : menuItems) {
			PVector pos = Tracker.mapPosition(item.position);
			PVector sz = Tracker.mapVelocity(new PVector(SELECTION_DISTANCE, SELECTION_DISTANCE));
			parent.ellipse((pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2, sz.x*wsize.x/2, sz.y*wsize.y/2);
			parent.text(item.name, (pos.x+1)*wsize.x/2, (pos.y+1)*wsize.y/2);
		}
	}
	
	@Override
	public void drawLaser(PApplet parent, People p) {
		super.drawLaser(parent,p);
		Laser laser = Laser.getInstance();
		laser.bgBegin();
		for(MenuItem item : menuItems) {
			laser.circle(item.position.x, item.position.y, SELECTION_DISTANCE);
//			PApplet.println("Circle at "+item.position)
		}
		laser.bgEnd();
	}
	
	@Override
	public void start() {
		super.start();
		menuItems.clear();
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
