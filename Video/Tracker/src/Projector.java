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
	ProjectorParameters params;

	public Projector(PApplet parent, int id, int width, int height) {
		// Create a syphon-based projector with given parameters
		this.id=id;
		params=ProjectorParameters.eh320ust();
		server=new SyphonServer(parent, "P"+id);
		PApplet.println("parent matrix:");
		parent.printMatrix();
		parent.printCamera();
		pcanvas = (PGraphicsOpenGL)parent.createGraphics(width, height,PConstants.P3D);
		testdecompose();

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
		pcanvas.resetMatrix();  // Set camera, modelview to identity
		pcanvas.ortho(0, pcanvas.width, -pcanvas.height, 0, 1e-10f,1e10f);
		// Not clear why this needs to be [-height,0] instead of [0,height]
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
					//PApplet.println("Cursor "+i+"/"+c.length+" for proj "+c[i].proj+" @ "+c[i].pos.x+", "+c[i].pos.y);
					//ttest(c[i].pos.x,c[i].pos.y,c[i].pos.z);
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
		PApplet.println("["+x+","+y+","+z+"]->["+pcanvas.screenX(x,y,z)+","+pcanvas.screenY(x,y,z)+","+pcanvas.screenZ(x,y,z)+"]");
	}

	public void setMatrix(PMatrix3D projmodelview) {

	}

	public PMatrix3D extractProj(PMatrix3D projmodelview) {
		PMatrix3D proj=new PMatrix3D(projmodelview);
		proj.apply(pcanvas.modelviewInv);
		return proj;
	}

	public void testdecompose() {
		// Test decompose by setting up a matrix from known parts, then try to decompose
		pcanvas.resetMatrix();
		PMatrix3D model=new PMatrix3D(pcanvas.modelview);
		PVector eye=new PVector(1f,2f,3f);
		PVector ref=new PVector(4f,7f,0f);
		PVector up=new PVector(0.1f,0.1f,1f);
		up.normalize();
		PVector aim=ref.copy();
		aim.sub(eye);
		aim.normalize();

		PApplet.print("eye="+eye);
		PApplet.print(", ref="+ref);
		PApplet.print(", up="+up);
		PApplet.println(", aim="+aim);

		pcanvas.camera(eye.x,eye.y,eye.z,ref.x,ref.y,ref.z,up.x,up.y,up.z);
		pcanvas.frustum(params.left,params.right,params.bottom,params.top,params.near,params.far);
		PApplet.println("preset model=");
		model.print();
		PApplet.println("preset camera=");
		pcanvas.camera.print();
		PApplet.println("preset proj=");
		pcanvas.projection.print();
		PApplet.println("preset modelview=");
		pcanvas.modelview.print();
		PApplet.println("preset projmodelview=");
		pcanvas.projmodelview.print();
		PMatrix3D dcamera=new PMatrix3D();
		decompose(pcanvas.projmodelview,model,new PMatrix3D(),dcamera,false);
		dcamera.invert();
		dcamera.preApply(pcanvas.camera);
		PApplet.println("camera*dcamera^-1: ");
		dcamera.print();
	}

	public void decomposeCamera(final PMatrix3D camera) {
		final boolean debug=true;

		// Decompose a camera view matrix into eye, aim, up
		PMatrix3D c=new PMatrix3D(camera);
		if (c.m33!=1f) {
			float rescale=1/c.m33;
			c.set(  c.m00*rescale, c.m01*rescale, c.m02*rescale, c.m03*rescale,
					c.m10*rescale, c.m11*rescale, c.m12*rescale, c.m13*rescale,
					c.m20*rescale, c.m21*rescale, c.m22*rescale, c.m23*rescale,
					c.m30*rescale, c.m31*rescale, c.m32*rescale, c.m33*rescale
					);
			if (debug) {
				PApplet.println("decomposeCamera: rescaling matrix by "+rescale+" -> ");
				c.print();
			}
		}
		if (Math.abs(new PVector(c.m00,c.m01,c.m02).mag()-1) >.01f)
			PApplet.println("Row 1 of camera matrix is not a unit vector");
		if (Math.abs(new PVector(c.m10,c.m11,c.m12).mag()-1) >.01f)
			PApplet.println("Row 2 of camera matrix is not a unit vector");
		if (Math.abs(new PVector(c.m20,c.m21,c.m22).mag()-1) >.01f)
			PApplet.println("Row 3 of camera matrix is not a unit vector");
		PVector aim = new PVector(-c.m20,-c.m21,-c.m22);

		// Compute the translation by pre-multiplying the camera matrix by the inverse of the untranslated camera matrix
		PMatrix3D untranslatedInv=new PMatrix3D(camera);
		untranslatedInv.m03=0; untranslatedInv.m13=0; untranslatedInv.m23=0; untranslatedInv.m33=1;
		untranslatedInv.invert();
		PMatrix3D translationMat=new PMatrix3D(camera);
		translationMat.preApply(untranslatedInv);
		PVector eye = new PVector(-translationMat.m03,-translationMat.m13,-translationMat.m23);

		// Find a reference point that determines the aim, preferable at z=0
		float aimscale=1f;
		if (aim.z!=0) {
			// Find a ref position with z=0
			aimscale=-eye.z/aim.z;
		}
		PVector ref=aim.copy();
		ref.mult(aimscale);
		ref.add(eye);

		// Read out up vector (but may be different than original, which didn't have to be orthogonal to other axes -- this one is)
		PVector up=new PVector(c.m10,c.m11,c.m12);

		if (debug) {
			PApplet.print("reconstructed aim="+aim);
			PApplet.print(", eye="+eye);
			PApplet.print(", ref="+ref);
			PApplet.println(", up="+up);
			pcanvas.pushMatrix();
			pcanvas.resetMatrix();
			pcanvas.camera(eye.x,eye.y,eye.z,ref.x,ref.y,ref.z,up.x,up.y,up.z);
			PApplet.println("recombine eye,ref,up: ");
			pcanvas.printCamera();
			pcanvas.popMatrix();
		}

	}

	public PMatrix3D screenToRelative(final PMatrix3D fulltransform) {
		// Convert a transform that maps from world -> screen (in pixels) to one the maps to relative screen position (-1:1)
		PMatrix3D S=new PMatrix3D(
				pcanvas.width/2, 0, 0, pcanvas.width/2,
				0,-pcanvas.height/2, 0, pcanvas.height/2,
				0, 0, 0.5f, 0.5f,
				0, 0, 0, 1);
		PMatrix3D SInv=new PMatrix3D(S);
		SInv.invert();
		PMatrix3D relative=new PMatrix3D(fulltransform);
		relative.preApply(SInv);
		return relative;
	}

	public void decompose(final PMatrix3D projmodelview, final PMatrix3D model, PMatrix3D proj, PMatrix3D camera, boolean zknown) {
		// Decompose a complete mapping from world to screen coordinates and model into separate projection, camera, model, screen normalization
		// PMV = P*C*M
		// Approach: Assume P based on known projector lenses, set MV to identity;  multiply by inverses to find C
		// Can then decompose C into eye, aim, up
		// If zknown is false, then the 3rd row and 3rd column of projmodelview is unknown and needs to be infered from camera matrix constraints
		proj=params.getProjection();
		PApplet.println("decompose: proj=");
		proj.print();

		PMatrix3D projinv=new PMatrix3D(proj);
		projinv.invert();
		PApplet.println("decompose: projinv=");
		projinv.print();

		PMatrix3D modelInv=new PMatrix3D(model);
		modelInv.invert();

		PMatrix3D pv=new PMatrix3D(projmodelview);
		pv.apply(modelInv);  // Get rid of effect of model

		if (!zknown) {
			// Reconstruct 3rd row and column of projmodelview from constraints
			// pv=P*C, where P is known and have constraints that: C(4,:)=[0 0 0 1], norm(C(k,1:3))=1.0 for k=1,2,3
			pv.m20=pv.m30*proj.m22/proj.m32;
			pv.m21=pv.m31*proj.m22/proj.m32;
			pv.m22=pv.m32*proj.m22/proj.m32;
			pv.m23=pv.m33*proj.m22/proj.m32+proj.m23;	
			pv.m32=-pv.m22*projinv.m32/projinv.m33;   // To make camera.m32=0
			pv.m02=Float.NaN;
			pv.m12=Float.NaN;
			PApplet.println("so far, pv=:");
			pv.print();		
			camera.set(pv);
			camera.preApply(projinv);
			PApplet.println("camera: ");
			camera.print();
			// Figure out what camera.m*2 should be to normalize the rows
			float norm0=camera.m00*camera.m00+camera.m01*camera.m01;
			float cm02=(norm0<1)?(float)Math.sqrt(1-norm0):0;   // Two possible solutions
			float norm1=camera.m10*camera.m10+camera.m11*camera.m11;
			float cm12=(norm1<1)?(float)Math.sqrt(1-norm1):0;  // Two possible solutions
			float norm2=camera.m20*camera.m20+camera.m21*camera.m21;
			float cm22=(norm2<1)?(float)Math.sqrt(1-norm2):0;
			pv.m02=cm02/projinv.m00;
			PApplet.println("norm0="+norm0+", cm02="+cm02+", pv.m02="+pv.m02);
			pv.m12=(cm12-projinv.m13*pv.m32)/projinv.m11;
			PApplet.println("so far2, pv=:");
			pv.print();	
		}
		camera.set(pv);
		camera.preApply(projinv);

		PApplet.println("camera: ");
		camera.print();

		decomposeCamera(camera);

		// Verify product (P*C*MV) == projmodelview
		PMatrix3D product=new PMatrix3D(proj);
		product.apply(camera);
		product.apply(model);

		PApplet.println("product:");
		product.print();
		float mx=maxdiff(projmodelview,product);
		if (mx>0.01f)
			PApplet.println("reconstructed product differs from original by up to "+mx);
	}

	public float maxdiff(PMatrix3D a, PMatrix3D b){
		float diffs[]={a.m00-b.m00,a.m01-b.m01,a.m02-b.m02,a.m03-b.m03,
				a.m10-b.m10,a.m11-b.m11,a.m12-b.m12,a.m13-b.m13,
				a.m20-b.m20,a.m21-b.m21,a.m22-b.m22,a.m23-b.m23,
				a.m30-b.m30,a.m31-b.m31,a.m32-b.m32,a.m33-b.m33};	
		float	mx=0;
		for (int i=0;i<diffs.length;i++) {
			mx=Math.max(mx,Math.abs(diffs[i]));
		}
		return mx;
	}
	
	public void setInvMatrix(PMatrix3D projmodelview, boolean zknown) {
		// Decompose to form independent projection and modelview matrices
		// If zknown is false, then the 3rd row and 3rd column of projmodelview are undetermined 
		// This results in 7 DOF, but the camera matrix has 7 constraints (form should be [ R : T ], where r is a rotation matrix -> 9 DOF in a 16 element matrix)

		PApplet.println("SetInvMatrix(");
		projmodelview.print();

		// PGraphics3D does additional scaling/centering of x,y after all the matrix transformations to get raw pixels
		// so we need to back that out of the target matrix
		PMatrix3D target=screenToRelative(projmodelview);
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
