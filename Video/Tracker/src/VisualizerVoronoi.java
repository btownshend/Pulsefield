import java.util.HashSet;
import java.util.List;

import processing.opengl.*;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PVector;
import processing.opengl.PGL;
import delaunay.*;

public class VisualizerVoronoi extends Visualizer {
	Triangle initialTriangle;
	final int initialSize=10000;
	
	VisualizerVoronoi(PApplet parent) {
		super();
        initialTriangle = new Triangle(
                new Pnt(-initialSize, -initialSize),
                new Pnt( initialSize, -initialSize),
                new Pnt(           0,  initialSize));
	}

	public void update(PApplet parent, Positions p) {
		// Update internal state
	}

	public void draw(PApplet parent, Positions allpos, PVector wsize) {
		PGL pgl=PGraphicsOpenGL.pgl;
		pgl.blendFunc(PGL.SRC_ALPHA, PGL.DST_ALPHA);
		pgl.blendEquation(PGL.FUNC_ADD);  
		parent.background(0, 0, 0);  
		parent.colorMode(PConstants.RGB, 255);

		super.draw(parent, allpos, wsize);

		parent.stroke(127);
		parent.strokeWeight(1);
		parent.fill(0);
		drawBorders(parent,true,wsize);

		// Create Delaunay triangulation
		Triangulation dt=new Triangulation(initialTriangle);
		
		for (Position p: allpos.positions.values()) {
			dt.delaunayPlace(new Pnt(p.origin.x,p.origin.y));
		}
		dt.delaunayPlace(new Pnt(-.5,.5));
		dt.delaunayPlace(new Pnt(.5,.2));
		// Draw Voronoi diagram
	       // Keep track of sites done; no drawing for initial triangles sites
        HashSet<Pnt> done = new HashSet<Pnt>(initialTriangle);
        for (Triangle triangle : dt)
            for (Pnt site: triangle) {
                if (done.contains(site)) continue;
                done.add(site);
                List<Triangle> list = dt.surroundingTriangles(site, triangle);
                parent.beginShape();
                for (Triangle tri: list) {
                    Pnt c=tri.getCircumcenter();
                    parent.vertex((float)((c.coord(0)+1)*wsize.x/2),(float)((c.coord(1)+1)*wsize.y/2));
                }
                parent.endShape();
            }

	}
}

