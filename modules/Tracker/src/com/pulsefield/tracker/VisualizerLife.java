package com.pulsefield.tracker;

import java.util.Arrays;

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
	float iterationsPerMinute; 

	// How many milliseconds to wait between cycles; calculated from iterationsPerMinute.
	long lifeDelayMillis;

	// The last time the life grid was updated.
	long lastLifeTimestamp;

	// Dimension of grid squares in meters.
	float gridSpacing = 0.3f;

	// Count of currently living organisms [0] and historically going back.
	int lifeHistoryLen = 100;
	int lifeHistory[];

	// Offsets to adjust from 0,0 centered coords so we can use a simple 2d array.
	float xoffset;
	float yoffset;

	int rows;
	int columns;

	int personCount = 0;

	PlaySound playSound = new PlaySound();
	Grid grid;
	
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
				return 0xFF006600;
			case DECAY2:
				return 0xFF112200;
			case NOLIFE:
				return 0xFF000000;
			default:
				break;
			}
			// Something is wrong if we fall through so return red to indicate
			// a problem.
			return 0xFFFF0000;
		}
	}

	VisualizerLife(PApplet parent) {
		// setupLifeGrid();
		final String songs[] = {"PR","EP","NG"};
		grid=new Grid(songs);
	}

	public void clickSound(int fileNumber) {
		String clickSoundFile;

		switch (fileNumber) {
		case 2:
			clickSoundFile = "./data/life/spaceshipG.au";
			break;
		case 1:
			clickSoundFile = "./data/life/spaceshipF.au";
			break;
		case 0:
		default:
			clickSoundFile = "./data/life/spaceshipEb.au";
			break;
		}

		//playSound.play(clickSoundFile);
	}

	private void checkFieldSizeChange() {
		PVector sz=Tracker.getFloorSize();

		if (((int) Math.ceil((sz.x/gridSpacing)) != this.columns) ||
		((int) Math.ceil((sz.y/gridSpacing)) != this.rows)) {
			setupLifeGrid();
		}
	}
	
	public void setupLifeGrid() {
		PVector sz=Tracker.getFloorSize();

		columns=(int) Math.ceil((sz.x/gridSpacing));
		rows=(int) Math.ceil((sz.y/gridSpacing));

		xoffset=Math.abs(Tracker.minx);
		yoffset=Math.abs(Tracker.miny);

		lifeGrid = new Cell[columns][rows];
		legGrid = new Cell[columns][rows];
		drawGrid = new Cell[columns][rows];

		lastLifeTimestamp = 0; // Setting to zero will trigger an immediate cycle then will set it for the next.

		// Initialize grids.
		for (int i=0; i < columns; i++) {
			for (int j=0; j < rows; j++) {
				lifeGrid[i][j] = Cell.NOLIFE;
				drawGrid[i][j] = Cell.NOLIFE;
				legGrid[i][j] = Cell.NOLIFE;
			}
		}

		// Initialize lifeHistory.
		lifeHistory = new int[lifeHistoryLen];
		Arrays.fill(lifeHistory, 3);

		// Start lifeGrid with some life in the middle for fun.
		int centerRow = (int) columns / 2;
		int centerCol = (int) rows / 2;
		lifeGrid[centerRow][centerCol - 1] = Cell.LIFE;
		lifeGrid[centerRow][centerCol] = Cell.LIFE;
		lifeGrid[centerRow][centerCol + 1] = Cell.LIFE;
	}

	public void start() {
		super.start();
		Laser.getInstance().setFlag("legs",0.0f);
		setupLifeGrid();
		grid.start();
	}

	public void stop() {
		playSound.stopAll();
		super.stop();
		grid.stop();
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
				if (checkx < 0 || checkx >= columns || checky < 0 || checky >= rows)
					continue;

				if (lifeGrid[checkx][checky].alive()) {
					neighborCount++;					
				}
			}
		}

		return neighborCount;
	}

	public void update(PApplet parent, People allpos) {

		iterationsPerMinute = MasterClock.gettempo();

		// Calculate delay until next cycle should take place; do this on every update as it's a fungible setting.
		lifeDelayMillis = (long) (1000 * (60.0 / iterationsPerMinute));

		// Clear legGrid and re-create it.
		for (int i = 0; i < columns; i++) {
			for (int j = 0; j < rows ; j++) {
				legGrid[i][j] = Cell.NOLIFE;
			}
		}
		for (Person person: allpos.pmap.values()) {
			for (Leg leg: person.legs) {
				int xposgrid = (int) ((leg.getOriginInMeters().x + xoffset) / gridSpacing);
				int yposgrid = (int) ((leg.getOriginInMeters().y + yoffset) / gridSpacing);

				// PApplet.println("Placing leg at " + xposgrid + "x" + yposgrid);

				if (xposgrid < 0 || xposgrid >= columns || yposgrid < 0 || yposgrid >= rows) {
					continue;
				}

				legGrid[xposgrid][yposgrid] = Cell.LEG;				
			}
		}

		personCount = allpos.pmap.size();

		// If it's time to iterate the lifeGrid, do it.
		if (System.currentTimeMillis() >= lastLifeTimestamp + lifeDelayMillis) {
			// Set next iteration time.
			lastLifeTimestamp = (long) System.currentTimeMillis();

			// Merge current legGrid into lifeGrid as we want a 'leg' to be treated as life while present.
			// Need to do this in a separate pass so that adjacent cells can be checked for the next pass.
			for (int i=0; i < columns; i++) {
				for (int j=0; j < rows; j++) {
					// If it was a leg before but not now, clear it. Only copy over native "life".
					if (lifeGrid[i][j] == Cell.LEG && legGrid[i][j] != Cell.LEG) {
						lifeGrid[i][j] = Cell.NOLIFE;
					} else if (legGrid[i][j] == Cell.LEG) {
						lifeGrid[i][j] = Cell.LEG;
					}
				}
			}

			// Push history record forward.
			for (int i=lifeHistoryLen - 1; i > 0 ; i--) {
				lifeHistory[i] = lifeHistory[i - 1];
			}

			int newCount = 0;

			// Calculate next generation of life.
			Cell newLife[][] = new Cell[columns][rows];
			for (int i=0; i < columns; i++) {
				for (int j=0; j < rows; j++) {
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
						newCount++;
						// No lifeCount modification here.
					} else if (present && neighbors > 3) {
						// Rule 3 : Any live cell with more than three live neighbours dies, as if by overpopulation.
						newLife[i][j] = Cell.DECAY;
					} else if (!present && neighbors == 3) {
						// Rule 4 : Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
						newLife[i][j] = Cell.LIFE;
						newCount++;
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
			lifeHistory[0] = newCount;
			// TODO: This would be a good spot to trigger a 'heartbeat' sound effect.

			float longAvgHist = avgArray(lifeHistory, 5);
			float shortAvgHist = avgArray(lifeHistory, 2);
			if (lifeHistory[0] > 0) {
				if (shortAvgHist > longAvgHist) {
					clickSound(2);
				} else if (shortAvgHist < longAvgHist) {
					clickSound(1);
				} else {
					clickSound(0);
				}
			}

		}

		// In every update (not just life iterations) re-calculate the drawGrid, overlap the current 
		// leg values over the calculated life for display.
		for (int i = 0; i < columns; i++) {
			for (int j = 0; j < rows ; j++) {
				if (legGrid[i][j] == Cell.LEG) {
					drawGrid[i][j] = Cell.LEG;	
				} else {
					drawGrid[i][j] = lifeGrid[i][j];
				}
			}
		}
		grid.update(allpos);
	}

	private float avgArray(int[] data, int count) {
		if (count < 1)
			return 0;

		int sum = 0;
		for (int i = 0; i < count; i++) {
			sum += data[i];
		}

		return (float) sum/count;
	}

	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g, p);		

		checkFieldSizeChange();
		
		// Set box border line, width and color.  We "pulse" the grid color for visual interest.
		g.strokeWeight(0.01f);

		// Based on the time since the last life iteration calculate a "pulsing" brightness
		// to use for the grid border.
		double elapsed = (double)(System.currentTimeMillis() - lastLifeTimestamp);
		// Calculate a range from 0 to 1.0 which increments then resets.
		double range = Math.min(1.0f, elapsed / lifeDelayMillis);
		// Map into -Pi to Pi.
		double rangePi = (Math.PI * range * 2) - Math.PI;
		// Map into sinusoidal value from 0.0 to 1.1.
		float brightnessMulitplier = (float)((Math.sin(rangePi) + 1.0f)/2.0f);

		// Suppress row and column edges.  Skip the edges as partial cells are ugly.
		int skipCells = 1;

		// Draw cells.
		for (int j = skipCells; j < ( rows - skipCells) ; j++) {
			for (int i = skipCells; i < (columns - skipCells); i++) {

				// Generate an interesting baseline grid pattern using the clock and then multiply
				// that based on the brightnessMulitplier to cause the pattern to pulse.
				// The magic numbers below are just trial-and-error to produce an interesting pattern.
				float cellBorderPattern = (float) ((Math.tan(avgArray(lifeHistory, 50) / 11.0 * Math.abs(((rows/2) - j))/30.0f * Math.abs((columns / 2) - i)/30.0f) + 1.0f)/2.0f);
				float cellBorderBrightness = 30.0f + ((100.0f * cellBorderPattern * brightnessMulitplier) * .7f);
				// PApplet.println("cellBorder : " + cellBorder + " at pct " + brightnessMulitplier);

				g.stroke(100.0f, cellBorderBrightness);

				float x1 = ((i) * gridSpacing) - xoffset;
				float y1 = ((j) * gridSpacing) - yoffset;

				// Set rect fill color based on cell value and draw it.
				g.fill(drawGrid[i][j].color());
				g.rect(x1, y1, gridSpacing, gridSpacing);

				// PApplet.println("filling " + i + "x" + j + "at" + x1 + "x" + y1 + "with" + drawGrid[i][j].color());
			}
		}
		//grid.draw(g,p);
	}

	public void drawLaser(PApplet parent, People p) {
		// TODO: Not sure what this laser stuff is!
	}
}

