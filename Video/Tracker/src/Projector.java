import codeanticode.syphon.SyphonServer;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PMatrix3D;
import processing.core.PVector;
import processing.opengl.PGraphicsOpenGL;

public class Projector {
	SyphonServer server;
	PGraphicsOpenGL pcanvas;
	int id;
	float throwRatio;
	PVector pos;
	PVector aim;
	float rotation;  // Rotation of projector in degrees -- normally 0

	PApplet p0;

	public Projector(PApplet parent, int id, int width, int height) {
		// Create a syphon-based projector with given parameters
		this.id=id;
		p0=parent;
		server=new SyphonServer(parent, "P"+id);
		PApplet.println("parent matrix:");
		parent.printMatrix();
		parent.printCamera();
		pcanvas = (PGraphicsOpenGL)parent.createGraphics(width, height,PConstants.P3D);

		PApplet.println("canvas created, matrix=");
		pcanvas.printMatrix();
		pcanvas.printCamera();
		
		PApplet.println("Current modelview matrix=");
		pcanvas.modelview.print();
		
		PApplet.println("Current proj matrix=");
		pcanvas.printProjection();
		PMatrix3D reconProj=extractProj(pcanvas.projmodelview);
		PApplet.println("Reconstructed projection matrix:");
		reconProj.print();


		ttest(1,2,0);
		throwRatio=0.25f;
		aim=Tracker.getFloorCenter();
		pos=new PVector(0f,-0.5f,1.5f); // Assume it is above/behind lidar
		rotation=0f;
		loadSettings();
		setTO();
	}

	public void setThrowRatio(float t) {
		throwRatio=t;
		setTO();
	}

	public void setPosition(float x, float y, float z) {
		// Set position and aiming of projector in floor coordinates
		// Convert to canvas coordinates
		pos=new PVector(x,y,z);
		setTO();
	}
	public void setAim(float aimx, float aimy) {
		aim=new PVector(aimx, aimy);
		setTO();
	}

	public void setRotation(float rotation) {
		this.rotation=rotation;
		setTO();
	}

	public void handleMessage(OscMessage msg) {
		PApplet.println("Projector message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");

		if (components.length==5 && components[3].equals("aim")&&msg.checkTypetag("f")) {
			if (components[4].equals("x"))
				aim.x=msg.get(0).floatValue();
			else if (components[4].equals("y"))
				aim.y=msg.get(0).floatValue();
			else
				PApplet.println("Bad projector aim message: "+msg.toString());
		} else if (components.length==5 && components[3].equals("pos")&&msg.checkTypetag("f")) {
			if (components[4].equals("x"))
				pos.x=msg.get(0).floatValue();
			else if (components[4].equals("y"))
				pos.y=msg.get(0).floatValue();
			else if (components[4].equals("z"))
				pos.z=msg.get(0).floatValue();
			else
				PApplet.println("Bad projector pos message: "+msg.toString());
		} else if (components.length==4 && components[3].equals("throwratio")&&msg.checkTypetag("f")) {
			if (msg.get(0).floatValue() >= 0.1f && msg.get(0).floatValue()<= 2f)
				throwRatio=msg.get(0).floatValue();
			else
				PApplet.println("Bad projector throwratio: "+msg.get(0).floatValue());
		} else if (components.length==4 && components[3].equals("rotation")&&msg.checkTypetag("f")) {
			rotation=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("save")) {
			saveSettings();
		} else 
			PApplet.println("Unknown projector Message: "+msg.toString());
		setTO();
	}

	private void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/proj/"+id+"/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/proj/"+id+"/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}

	public void setTO() {
		setTOValue("aim/x",aim.x,"%.2f");
		setTOValue("aim/y",aim.y,"%.2f");
		setTOValue("pos/x",pos.x,"%.2f");
		setTOValue("pos/y",pos.y,"%.2f");
		setTOValue("pos/z",pos.z,"%.2f");
		setTOValue("throwratio",throwRatio,"%.4f");
		setTOValue("rotation",rotation,"%.4f");
	}

	public void saveSettings() {
		PApplet.println("Projector.saveSettings("+id+")");
		Config.setFloat("proj"+id, "aim.x", aim.x);
		Config.setFloat("proj"+id, "aim.y", aim.y);
		Config.setFloat("proj"+id, "pos.x", pos.x);
		Config.setFloat("proj"+id, "pos.y", pos.y);
		Config.setFloat("proj"+id, "pos.z", pos.z);
		Config.setFloat("proj"+id, "rotation", rotation);
		Config.setFloat("proj"+id, "throwratio", throwRatio);
	}

	public void loadSettings() {
		PApplet.println("Projector.loadSettings("+id+")");
		aim.x=Config.getFloat("proj"+id, "aim.x", aim.x);
		aim.y=Config.getFloat("proj"+id, "aim.y", aim.y);
		pos.x=Config.getFloat("proj"+id, "pos.x", pos.x);
		PApplet.println("pos.x="+pos.x);
		pos.y=Config.getFloat("proj"+id, "pos.y", pos.y);
		pos.z=Config.getFloat("proj"+id, "pos.z", pos.z);
		rotation=Config.getFloat("proj"+id, "rotation", rotation);
		throwRatio=Config.getFloat("proj"+id, "throwratio", throwRatio);
	}
	public void render(PGraphics canvas) {
		// Send the given canvas to the projector
		// Assumes the canvas is on the floor (i.e. z=0)
		// Its center corresponds to Tracker.getFloorCenter() and is scaled by Tracker.getPixelsPerMeter()

		pcanvas.beginDraw();
		if (false) {
		float aspect=pcanvas.width*1.0f/pcanvas.height;
		float vfov=(float)(2f*Math.atan(1f/(aspect*throwRatio*2)));

		pcanvas.pushMatrix();
		pcanvas.perspective(vfov, aspect, 1f, 1e10f);
		// Projector image needs to be flipped in z direction to align correctly
		pcanvas.camera(pos.x,pos.y,pos.z,aim.x,aim.y,0.0f,(float)Math.sin(rotation*PConstants.PI/180),1.0f*(float)Math.cos(rotation*PConstants.PI/180),0.0f);
		//PApplet.println("Projector "+id+": pos=["+pos+"], aspect="+aspect+", vfov="+vfov*180f/Math.PI+" degrees.");
		}
		pcanvas.background(0);

		pcanvas.imageMode(PConstants.CENTER);
		PVector center=Tracker.getFloorCenter();
		pcanvas.image(canvas,center.x,center.y,canvas.width/Tracker.getPixelsPerMeter(),canvas.height/Tracker.getPixelsPerMeter());
		//pcanvas.popMatrix();  // Back to normal coords

		pcanvas.pushMatrix();
		pcanvas.pushProjection();
		pcanvas.resetMatrix();
		pcanvas.ortho();

		pcanvas.translate(-pcanvas.width/2, -pcanvas.height/2);

		ProjCursor c[]=Tracker.cursors;
		if (c!=null) {
			pcanvas.strokeWeight(2.0f);
			pcanvas.fill(255,0,0);
			pcanvas.stroke(0,255,0,255);
			pcanvas.ellipseMode(PConstants.CENTER);
			for (int i=0;i<c.length;i++) {
				if (c[i]==null)
					continue;
				if (c[i].proj+1==id) {
					final float CURSORLEN=50f;
					//PApplet.println("Cursor for proj "+c[i].proj+" @ "+c[i].pos.x+", "+c[i].pos.y);

					pcanvas.ellipse(c[i].pos.x, c[i].pos.y, CURSORLEN/2, CURSORLEN/2);
					pcanvas.line(c[i].pos.x-CURSORLEN,c[i].pos.y,c[i].pos.x+CURSORLEN,c[i].pos.y);
					pcanvas.line(c[i].pos.x,c[i].pos.y-CURSORLEN,c[i].pos.x,c[i].pos.y+CURSORLEN);
				}
			}
		}
		pcanvas.popProjection();
		pcanvas.popMatrix();
		pcanvas.endDraw();
		server.sendImage(pcanvas);
	}

	private void ttest(float x, float y, float z) {
		PApplet.println("["+x+","+y+","+z+"]->["+pcanvas.screenX(x,y,z)+","+pcanvas.screenY(x,y,z)+","+pcanvas.screenZ(x,y,z)+"]->"+
				"["+p0.screenX(x,y,z)+","+p0.screenY(x,y,z)+","+p0.screenZ(x,y,z)+"]");
	}

	public void setMatrix(PMatrix3D projmodelview) {

	}

	public PMatrix3D extractProj(PMatrix3D projmodelview) {
		PMatrix3D proj=new PMatrix3D(projmodelview);
		proj.apply(pcanvas.modelviewInv);
		return proj;
	}

	public void setInvMatrix(PMatrix3D projmodelview) {
		// Decompose to form independent projection and modelview matrices
		//pcanvas.translate(1.0f, 2.0f, 3.0f);
		//pcanvas.scale(5f,6f,7f);
		PApplet.println("SetInvMatrix(");
		projmodelview.print();
		
		// PGrraphics3D does additional scaling/centering of x,y after all the matrix transformations to get raw pixels
		// so we need to back that out of the target matrix
		PMatrix3D target=new PMatrix3D(projmodelview);
		PMatrix3D finalscale=new PMatrix3D(
				pcanvas.width/2, 0, 0, pcanvas.width/2,
				0,-pcanvas.height/2, 0, pcanvas.height/2,
				0, 0, 0.5f, 0.5f,
				0, 0, 0, 1);
		PMatrix3D unscale=new PMatrix3D(finalscale);
		unscale.invert();
		PApplet.println("unscale: ");
		unscale.print();
		target.preApply(unscale);
		PApplet.println("target: ");
		target.print();
		
		PMatrix3D newproj=extractProj(target);
		PApplet.println("Extracted new proj matrix");
		newproj.print();

		pcanvas.setProjection(newproj);
		PApplet.println("Updated proj matrix=");
		pcanvas.printProjection();

		PApplet.println("projmodelview Matrix after applying transform");
		pcanvas.projmodelview.print();

		ttest(0,0,0);
		ttest(0,2,0);
		ttest(-2,2,0);
		ttest(2,2,0);
		ttest(0,0,1);
		//assert(false);
	}
}
