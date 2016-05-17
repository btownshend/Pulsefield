import codeanticode.syphon.SyphonServer;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;

public class Projector {
	SyphonServer server;
	PGraphics pcanvas;
	int id;
	
	public Projector(PApplet parent, int id, int width, int height) {
	// Create a syphon-based projector with given parameters
		this.id=id;
		server=new SyphonServer(parent, "P"+id);
		pcanvas = parent.createGraphics(width, height, PConstants.P3D);
	}
	
	public void setPosition(float x, float y, float z, float aimx, float aimy) {
	// Set position and aiming of projector in floor coordinates
		// Convert to canvas coordinates
		
		pcanvas.camera(x,y,z,aimx,aimy,0.0f,0.0f,0.0f,1.0f);
	}
	
	public void setPerspective(float vfov, float aspect) {
		pcanvas.perspective(vfov, aspect, 1f, 1e10f);
	}
	
	public void render(PGraphics canvas) {
		// Send the given canvas to the projector
		pcanvas.beginDraw();
		pcanvas.fill(127);
		//pcanvas.scale(0.5f);

		if (id==1) {
			pcanvas.translate(-canvas.width/2,-canvas.height/2);
			pcanvas.rotateY(PApplet.radians(10));
			pcanvas.translate(0, 0,10);
			pcanvas.translate(pcanvas.width/2,pcanvas.height/2);
		} else {
			pcanvas.camera(canvas.width/2,-canvas.height/4,canvas.height/4,canvas.width/2,canvas.height/2,0f,0,1,0);
		}

		pcanvas.image(canvas,0,0);
		pcanvas.endDraw();
		server.sendImage(pcanvas);
	}
}
