import codeanticode.syphon.SyphonServer;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

public class Projector {
	SyphonServer server;
	PGraphics pcanvas;
	int id;
	float throwRatio;
	PVector pos;
	PVector aim;
	
	public Projector(PApplet parent, int id, int width, int height) {
	// Create a syphon-based projector with given parameters
		this.id=id;
		server=new SyphonServer(parent, "P"+id);
		pcanvas = parent.createGraphics(width, height, PConstants.P3D);
		this.throwRatio=0.25f;
		this.aim=Tracker.getFloorCenter();
		this.pos=new PVector(0f,-0.5f,1.5f); // Assume it is above/behind lidar
	}
	
	public void setThrowRatio(float t) {
		throwRatio=t;
	}
	
	public void setPosition(float x, float y, float z) {
	// Set position and aiming of projector in floor coordinates
		// Convert to canvas coordinates
		pos=new PVector(x,y,z);
	}
	public void setAim(float aimx, float aimy) {
		aim=new PVector(aimx, aimy);
	}

	public void render(PGraphics canvas) {
		// Send the given canvas to the projector
		pcanvas.beginDraw();
		float aspect=1920f/1080f;
		float vfov=(float)(2f*Math.atan(1f/(aspect*throwRatio*2)));
		pcanvas.perspective(vfov, aspect, 1f, 1e10f);
		// Flip the projector "up" direction so that this lines up
		// I think this is needed because the lidar coordinate system implies z is down
		// Also, the image operation 
		pcanvas.camera(pos.x,pos.y,pos.z,aim.x,aim.y,0.0f,0.0f,0.0f,-1.0f);
		//PApplet.println("Projector "+id+": pos=["+pos+"], aspect="+aspect+", vfov="+vfov*180f/Math.PI+" degrees.");

		pcanvas.background(0);

		pcanvas.imageMode(PConstants.CENTER);
		PVector center=Tracker.getFloorCenter();
		PVector sz=Tracker.getFloorSize();
		pcanvas.image(canvas,center.x,center.y,sz.x,sz.y);
		pcanvas.endDraw();
		server.sendImage(pcanvas);
	}
}
