package com.pulsefield.tracker;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;

// Implements conway's game of life on a grid.  One (or more) legs on a square count as a "living" cell.
// "leg" creatures (those present because of a real person) can't be killed (but will be marked with a unique color).
// To add a bit more visual interest there is also a 'decay' where 'dead' cells darken in subsequent cycles.

// TODO: Music / sound.  Perhaps play different sounds based on the aggregate birth/death rate?  Or
// maybe look for inflection points in this rate and trigger a transition.

public class VisualizerLife extends Visualizer {
	Cell lifeGrid[][];
	Cell drawGrid[][];
	Cell legGrid[][];

	// Number of 'life' game iterations per minute (higher number is faster pace).
	int iterationsPerMinute = 155; 

	// Dimension of grid squares in meters.
	float gridSpacing = 0.12f; 
	
	long nextLifeTimestamp;
	
	// Offsets to adjust from 0,0 centered coords so we can use a simple 2d array.
	float xoffset;
	float yoffset;

	int rows;
	int columns;
	
	public enum Cell {
		NOLIFE,
		LIFE,
		LEG,
		DECAY,
		DECAY2;
		
		// Do we treat this cell as "alive" for growth/death purposes?
		public boolean alive() {
			switch (this) {
			case LIFE:
				return true;
			case LEG:
				return true;
			default:
				break;
			}
			return false;
		}

		public int color() {
			switch (this) {
			case LIFE:
				return 0xFF00DD00;
			case LEG:
				return 0xFF4444FF;
			case DECAY:
				return 0xFF008800;
			case DECAY2:
				return 0xFF113300;
			case NOLIFE:
			default:
				break;
			}
			return 0xFF000000;
		}
	}

	VisualizerLife(PApplet parent) {
		setupLifeGrid();
	}

	public void setupLifeGrid() {
		PApplet.println("setupLifeGrid()");
		PVector sz=Tracker.getFloorSize();

		rows=(int) Math.ceil((sz.x/gridSpacing));
		columns=(int) Math.ceil((sz.y/gridSpacing));
		
		xoffset=Math.abs(Tracker.minx);
		yoffset=Math.abs(Tracker.miny);

		lifeGrid = new Cell[rows][columns];
		legGrid = new Cell[rows][columns];
		drawGrid = new Cell[rows][columns];

		nextLifeTimestamp = 0; // Immediately run and set the timer on the next iteration.
		
		// Initialize grids.
		for (int i=0; i < rows; i++) {
			for (int j=0; j < columns; j++) {
				lifeGrid[i][j] = Cell.NOLIFE;
				drawGrid[i][j] = Cell.NOLIFE;
				legGrid[i][j] = Cell.NOLIFE;
			}
		}
		
		// Start lifeGrid with some life in the middle for fun.
		int centerRow = (int) rows / 2;
		int centerCol = (int) columns / 2;
		lifeGrid[centerRow][centerCol - 1] = Cell.LIFE;
		lifeGrid[centerRow][centerCol] = Cell.LIFE;
		lifeGrid[centerRow][centerCol + 1] = Cell.LIFE;
	}

	public void start() {
		super.start();
		Laser.getInstance().setFlag("body",0.0f);
		Laser.getInstance().setFlag("legs",0.0f);
		setupLifeGrid();
	}

	public void stop() {
		super.stop();
	}

	// Count the number of neighbors at a specified coordinate.	
	private int lifeCellNeighbors(int x, int y) {
		int neighborCount = 0;

		// Walk around the current cell and check for alive neighbors.
		for (int i=-1; i <= 1; i++ ) {
			for (int j=-1; j <=1 ; j++) {

				// Don't count the cell itself.
				if (i==0 && j==0)
					continue;

				int checkx = x+i;
				int checky = y+j;

				// Check bounds for the probed cell.
				if (checkx < 0 || checkx >= rows || checky < 0 || checky >= columns)
					continue;

				if (lifeGrid[checkx][checky].alive()) {
					neighborCount++;					
				}
			}
		}
		
		return neighborCount;
	}

	public void update(PApplet parent, People allpos) {
		// Clear legGrid and re-create it.
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns ; j++) {
				legGrid[i][j] = Cell.NOLIFE;
			}
		}
		for (Person person: allpos.pmap.values()) {
			for (Leg leg: person.legs) {
				int xposgrid = (int) ((leg.getOriginInMeters().x + xoffset) / gridSpacing);
				int yposgrid = (int) ((leg.getOriginInMeters().y + yoffset) / gridSpacing);

				// PApplet.println("Placing leg at " + xposgrid + "x" + yposgrid);

				if (xposgrid < 0 || xposgrid >= rows || yposgrid < 0 || yposgrid >= columns) {
					continue;
				}
				
				legGrid[xposgrid][yposgrid] = Cell.LEG;				
			}
		}

		// If it's time to iterate the lifeGrid, do it.
		if (System.currentTimeMillis() >= nextLifeTimestamp) {
			// Set next iteration time.
			nextLifeTimestamp = (long) (System.currentTimeMillis() + (1000 * ((60.0 / iterationsPerMinute))));

			// Merge current legGrid into lifeGrid as we want a 'leg' to be treated as life while present.
			// Need to do this in a separate pass so that adjacent cells can be checked for the next pass.
			for (int i=0; i < rows; i++) {
				for (int j=0; j < columns; j++) {
					// If it was a leg before but not now, clear it. Only copy over native "life".
					if (lifeGrid[i][j] == Cell.LEG && legGrid[i][j] != Cell.LEG) {
						lifeGrid[i][j] = Cell.NOLIFE;
					} else if (legGrid[i][j] == Cell.LEG) {
						lifeGrid[i][j] = Cell.LEG;
					}
				}
			}

			// Calculate next generation of life.
			Cell newLife[][] = new Cell[rows][columns];
			for (int i=0; i < rows; i++) {
				for (int j=0; j < columns; j++) {
					// Get count of live neighbors at this cell.
					int neighbors = lifeCellNeighbors(i, j);
					boolean present = lifeGrid[i][j].alive();
					
					if (present && neighbors < 2) {
						// "Rules" from https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life					
						// Rule 1 : Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
						newLife[i][j] = Cell.DECAY;
					} else if (present && neighbors >= 2 && neighbors <= 3) {
						// Rule 2 : Any live cell with two or three live neighbours lives on to the next generation.
						newLife[i][j] = Cell.LIFE;
					} else if (present && neighbors > 3) {
						// Rule 3 : Any live cell with more than three live neighbours dies, as if by overpopulation.
						newLife[i][j] = Cell.DECAY;
					} else if (!present && neighbors == 3) {
						// Rule 4 : Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
						newLife[i][j] = Cell.LIFE;
					} else {
						// Further any existing decay (note: new cells can override decay).
						if (lifeGrid[i][j] == Cell.DECAY) {
							newLife[i][j] = Cell.DECAY2;
						} else {
							newLife[i][j] = Cell.NOLIFE;
						}
					}
				}	
			}
			lifeGrid = newLife;
			
			// TODO: This would be a good spot to trigger a 'heartbeat' sound effect.
		}

		// In every update (not just life iterations) re-calculate the drawGrid, overlap the current 
		// leg values over the calculated life for display.
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns ; j++) {
				if (legGrid[i][j] == Cell.LEG) {
					drawGrid[i][j] = Cell.LEG;	
				} else {
					drawGrid[i][j] = lifeGrid[i][j];
				}
			}
		}
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);		
		
		// Set box border line width and color.
		g.strokeWeight(0.01f);
		g.stroke(0xFF555555);
		
		// Draw cells.
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns ; j++) {
				float x1 = ((i) * gridSpacing) - xoffset;
				float x2 = ((i+1) * gridSpacing) - xoffset;
				float y1 = ((j) * gridSpacing) - yoffset;
				float y2 = ((j+1) * gridSpacing) - yoffset;
				
				g.rect(x1, y1, (x2-x1), (y2-y1));
				g.fill(drawGrid[i][j].color());
			}
		}
	}

	public void drawLaser(PApplet parent, People p) {
		// TODO: Not sure what this laser stuff is!
	}
}

