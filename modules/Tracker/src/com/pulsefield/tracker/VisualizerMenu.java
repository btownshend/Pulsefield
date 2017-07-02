package com.pulsefield.tracker;
import java.util.HashSet;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
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
	int selectedCount;
	PImage menuImage;
	
	/** How many menu items to display at once (includes "next page" item). */
	static final int ITEMS_PER_SCREENFUL = 25;
	
	/** Minimum distance from item to person/other item when initializing (in meters). */
	static final float MIN_DISTANCE = 1.0f;
	
	/** How far a person needs to be to be able to select the menu item (in meters). */
	static final float SELECTION_DISTANCE = 0.3f;
	
	/** Text height (in meters) **/
	static final float TEXT_HEIGHT = 0.15f;
	
	static final float hotSpotRadius=0.3f;   // Radius of hot spot in meters
	static final PVector hotSpot = new PVector(0f,0f);  // Location of hotspot (in meters) (updated when drawn)
	static final int HOTSPOTCOUNT=30;  // Number of frames to trigger
	
	/** Cursor radius (in meters) **/
	static final float CURSOR_RADIUS = 0.3f;

	/** Set of items currently being displayed. */
	HashSet<MenuItem> menuItems = new HashSet<MenuItem>();
	PShape cursor;
	static final String CURSOR="handCursor.svg";
	
	VisualizerMenu(PApplet parent) {
		super();
		selectingPerson=-1;
		selectedCount=0;
		cursor=parent.loadShape(Tracker.SVGDIRECTORY+CURSOR);
	}
	
	boolean hotSpotCheck(PApplet parent, People people) {
		for(Person p : people.pmap.values()) {
			PVector location = p.getOriginInMeters();
			if(PVector.sub(location, hotSpot).mag() < hotSpotRadius) {
				selectingPerson=p.id;
				selectedCount+=1;
				logger.fine("Person hit hot spot: "+selectingPerson);
				return selectedCount>=HOTSPOTCOUNT;
			}
		}
		selectedCount=0;
		return false;
	}
	
	void hotSpotDraw(PGraphics g) {
		//		if (appleShape==null)
		//		appleShape=g.loadShape(Tracker.SVGDIRECTORY+"apple.svg");
		if (menuImage==null)
			menuImage=Tracker.theTracker.loadImage("menu/menu.png");
		g.imageMode(PConstants.CENTER);
		g.tint(255,255);
		hotSpot.x=(Tracker.minx+Tracker.maxx)/2;
		hotSpot.y=Tracker.miny+hotSpotRadius+0.05f;
		Visualizer.drawImage(g,menuImage, hotSpot.x,hotSpot.y,hotSpotRadius*2,hotSpotRadius*2*menuImage.height/menuImage.width);
		if (selectedCount > 0 && selectedCount<=HOTSPOTCOUNT) {
			g.fill(0);
			g.textAlign(PConstants.CENTER,PConstants.CENTER);
			Visualizer.drawText(g, 0.4f, Integer.toString(HOTSPOTCOUNT-selectedCount), hotSpot.x, hotSpot.y);
		}
	}

	/** Next visualizer to serve up.  Call getNextVisualizerIndexSet */
	private int nextVisualizerIndex = 0;
	
	/** Return next visualizer to serve up.  -1 indicates the special 'more' menu category */
	private HashSet<Integer> getNextVisualizerIndexSet() {
		HashSet<Integer> results = new HashSet<Integer>();
		while(true) {
			logger.fine("getNextSet: "+nextVisualizerIndex+": "+Tracker.vis[nextVisualizerIndex].selectable);
			if (Tracker.vis[nextVisualizerIndex].selectable)
				results.add(nextVisualizerIndex);
			nextVisualizerIndex++;
			// If we reached the end of the list, we stop adding.
			if (nextVisualizerIndex >= Tracker.vis.length) {
				nextVisualizerIndex = 0;
				break;
			}
			// Want to leave one slot for more options
			if (results.size() >= ITEMS_PER_SCREENFUL - 1) {
				results.add(-1);
				break;
			}
		}
		logger.fine("Result="+results);
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
			int nattempts=100;
			while(nattempts>0) {
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
				if(Float.isNaN(minDistance) || minDistance > MIN_DISTANCE*nattempts/100) {
					menuItems.add(new MenuItem(proposedPosition, index));
					allPositions.add(proposedPosition);
					break;
				}
				nattempts-=1;
			}
			if (nattempts==0) {
				logger.warning("Unable to find a location for "+index);
			}
		}
		logger.fine("Found all positions");
	}
	
	@Override
	public void update(PApplet parent, People p) {
		// Initialize menu items if necessary
		if(menuItems.isEmpty()) {
			initializeItems(p);
		}
		
		if (!p.pmap.containsKey(selectingPerson)) {
			// Person has left
			logger.fine("Selecting person "+selectingPerson+" has left (pmap contains "+p.pmap.size()+" people)");
			((Tracker)parent).setapp(3);
		}
		Person ps=p.pmap.get(selectingPerson);
		if (ps==null) {
			logger.fine("Selecting person "+selectingPerson+" not found");
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
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);
		if (p.pmap.isEmpty())
			return;
		
		g.ellipseMode(PConstants.CENTER);

		g.stroke(0xffffffff);
		g.textAlign(PConstants.CENTER, PConstants.CENTER);
		for(MenuItem item : menuItems) {
			g.fill(0xff000000);
			g.ellipse(item.position.x, item.position.y, 2*SELECTION_DISTANCE,2*SELECTION_DISTANCE );
			g.fill(0xffffffff);
			drawText(g,TEXT_HEIGHT,item.name, item.position.x, item.position.y);
		}
		// Draw cursor for selecting person
		Person ps=p.get(selectingPerson);
		float scale=Math.min(CURSOR_RADIUS*2/cursor.width,CURSOR_RADIUS*2/cursor.height);
		int c=ps.getcolor();
		g.fill(c,255);
		g.stroke(c,255);
//		logger.fine("Drawing cursor with scaling="+scale);
		// cursor seems shifted
		g.shapeMode(PConstants.CENTER);
		Visualizer.drawShape(g, cursor,ps.getOriginInMeters().x, ps.getOriginInMeters().y,cursor.width*scale,cursor.height*scale);
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
//			logger.fine("Circle at "+item.position)
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
		 name = visualizer == -1 ? "More" : Tracker.vis[visualizer].name;
	}
}
