import codeanticode.syphon.SyphonServer;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PMatrix2D;
import processing.core.PMatrix3D;
import processing.core.PVector;
import processing.opengl.PGraphicsOpenGL;


public class Projector {
	SyphonServer server;
	PGraphicsOpenGL pcanvas;
	int id;
	PVector pos;
	ProjectorParameters params;
	private final int debug=1;
	PMatrix3D camview=new PMatrix3D();
	PMatrix3D w2s=new PMatrix3D();
	PMatrix3D s2w=new PMatrix3D();
	PVector bounds[];


	public Projector(PApplet parent, int id, int width, int height) {
		// Create a syphon-based projector with given parameters
		this.id=id;
		params=ProjectorParameters.eh320ust();
		server=new SyphonServer(parent, "P"+id);
		if (debug>5) {
			PApplet.println("P"+id+" parent matrix:");
			parent.printMatrix();
			parent.printCamera();
		}
		pcanvas = (PGraphicsOpenGL)parent.createGraphics(width, height,PConstants.P3D);
		//testdecompose();

		if (debug>5) {
			matprint("Projector: Current camera matrix",pcanvas.camera);
			matprint("Projector: Current modelview matrix",pcanvas.modelview);
			matprint("Projector: Current proj matrix",pcanvas.projection);
			ttest(1,2,0);
		}
		// Make sure canvas is fully initialized before resetting its matrices
		pcanvas.beginDraw();
		pcanvas.endDraw();
		// Load settings from JSON
		loadSettings();
	}

	public void setPosition(PVector newpos) {
		// Set position and aiming of projector in floor coordinates
		// Convert to canvas coordinates
		pos=newpos;
	}

	public void handleMessage(OscMessage msg) {
		if (debug>0)
			PApplet.println("Projector message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");

		if (components.length==4 && components[3].equals("load")) {
			loadSettings();
		} else if (components.length==4 && components[3].equals("save")) {
			saveSettings();
		} else 
			PApplet.println("Unknown projector Message: "+msg.toString());
	}

	public void saveSettings() {
		PApplet.println("Projector.saveSettings("+id+")");
		Config.setVec("proj"+id, "pos", pos);
		Config.setMat("proj"+id,"s2w",s2w);
		Config.setMat("proj"+id,"w2s",w2s);
		Config.setMat("proj"+id,"camview",camview);
		Config.setMat("proj"+id,"projection",params.projMatrix);
	}

	public void loadSettings() {
		PApplet.println("Projector.loadSettings("+id+")");
		setPosition(Config.getVec("proj"+id,"pos",pos));
		setCameraView(Config.getMat("proj"+id,"camview",camview));
		params.setProjection(Config.getMat("proj"+id, "projection", params.projMatrix));
		setScreen2World(Config.getMat("proj"+id,"s2w",s2w));
		setWorld2Screen(Config.getMat("proj"+id,"w2s",w2s));
		ttest(0,0,0);
	}

	public void render(PGraphics canvas, PGraphics mask) {
		// Send the given canvas to the projector
		// Assumes the canvas is on the floor (i.e. z=0)
		// Its center corresponds to Tracker.getFloorCenter() and is scaled by Tracker.getPixelsPerMeter()
		pcanvas.beginDraw();
		pcanvas.background(0);
		pcanvas.imageMode(PConstants.CENTER);
		PVector center=Tracker.getFloorCenter();
		pcanvas.blendMode(PConstants.ADD);
		pcanvas.image(canvas,center.x,center.y,canvas.width/Tracker.getPixelsPerMeter(),canvas.height/Tracker.getPixelsPerMeter());
		pcanvas.blendMode(PConstants.MULTIPLY);
		pcanvas.smooth();
		pcanvas.image(mask,center.x,center.y,canvas.width/Tracker.getPixelsPerMeter(),canvas.height/Tracker.getPixelsPerMeter());
		pcanvas.blendMode(PConstants.ADD);
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
		PApplet.println("["+x+","+y+","+z+"]->["+pcanvas.screenX(x,y,z)+","+pcanvas.screenY(x,y,z)+","+pcanvas.screenZ(x,y,z)+"]; expect "+world2screen(new PVector(x,y,z)));
	}

	public PVector screen2world(PVector s) {
		// Map a vector in screen coords to one in world coords
		float x=s2w.m00*s.x+s2w.m01*s.y+s2w.m03;
		float y=s2w.m10*s.x+s2w.m11*s.y+s2w.m13;
		float w=s2w.m30*s.x+s2w.m31*s.y+s2w.m33;
		return new PVector(x/w,y/w);  // x is flipped
	}

	public PVector world2screen(PVector world) {
		// Map a vector in screen coords to one in world coords
		float x=w2s.m00*world.x+w2s.m01*world.y+w2s.m03;
		float y=w2s.m10*world.x+w2s.m11*world.y+w2s.m13;
		float w=w2s.m30*world.x+w2s.m31*world.y+w2s.m33;
		return new PVector(x/w,y/w);
	}

	public void computeBounds() {
		bounds=new PVector[4];  // Bounds of projector image in world coordinates
		bounds[0]=screen2world(new PVector(0,0));
		bounds[1]=screen2world(new PVector(pcanvas.width,0));
		bounds[2]=screen2world(new PVector(pcanvas.width,pcanvas.height));
		bounds[3]=screen2world(new PVector(0,pcanvas.height));

		PApplet.print("Projector "+id+" bounds:");
		for (int i=0;i<bounds.length;i++) {
			PVector recompute=world2screen(bounds[i]);
			PApplet.print(bounds[i]+"->"+recompute);
		}
		PApplet.println("");	
	}

	public void setScreen2World(PMatrix3D s2wMat) {
		s2w=s2wMat;
		computeBounds();
	}

	@Deprecated
	public void testdecompose() {
		// Test decompose by setting up a matrix from known parts, then try to decompose
		pcanvas.resetMatrix();
		PMatrix3D model=new PMatrix3D(pcanvas.modelview);
		PVector eye=new PVector(1f,2f,3f);
		PVector ref=new PVector(4f,7f,0f);
		PVector up=new PVector(0.1f,0.1f,1f);up.normalize();
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
		matprint("testdecompose: preset model",model);
		matprint("testdecompose: preset camera",pcanvas.camera);
		matprint("testdecompose: preset proj",pcanvas.projection);
		matprint("testdecompose: preset modelview",pcanvas.modelview);
		matprint("testdecompose: preset projmodelview",pcanvas.projmodelview);
		PMatrix3D dcamera=new PMatrix3D();
		decompose(pcanvas.projmodelview,model,new PMatrix3D(),dcamera,false);
		dcamera.invert();
		dcamera.preApply(pcanvas.camera);
		matprint("testdecompose: camera*dcamera^-1",dcamera);
	}

	public void decomposeCamera(final PMatrix3D camera) {
		// Decompose a camera view matrix into eye, aim, up
		PMatrix3D c=new PMatrix3D(camera);
		if (c.m33!=1f) {
			float rescale=1/c.m33;
			c.set(  c.m00*rescale, c.m01*rescale, c.m02*rescale, c.m03*rescale,
					c.m10*rescale, c.m11*rescale, c.m12*rescale, c.m13*rescale,
					c.m20*rescale, c.m21*rescale, c.m22*rescale, c.m23*rescale,
					c.m30*rescale, c.m31*rescale, c.m32*rescale, c.m33*rescale
					);
			if (debug>2)
				matprint("decomposeCamera: rescaling matrix by "+rescale,c);
		}
		// Check if camera view matrix is of the correct form [ R : T ]
		if (Math.abs(new PVector(c.m00,c.m01,c.m02).mag()-1) >.01f)
			PApplet.println("Row 1 of camera matrix is not a unit vector");
		if (Math.abs(new PVector(c.m10,c.m11,c.m12).mag()-1) >.01f)
			PApplet.println("Row 2 of camera matrix is not a unit vector");
		if (Math.abs(new PVector(c.m20,c.m21,c.m22).mag()-1) >.01f)
			PApplet.println("Row 3 of camera matrix is not a unit vector");
		if (c.m30!= 0 || c.m31!=0 || c.m32!=0 || c.m33!=1) 
			PApplet.println("Row 4 of camera matrix is not [0 0 0 1]");

		PVector aim = new PVector(-c.m20,-c.m21,-c.m22);

		// Compute the translation by pre-multiplying the camera matrix by the inverse of the untranslated camera matrix
		PMatrix3D camInv=new PMatrix3D(camera);
		camInv.invert();
		matprint("inv(camera)",camInv);
		PVector eye=new PVector(camInv.m03,camInv.m13,camInv.m23);


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

		if (debug>1) {
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
		// Z coords are mapping from [-1:1] to [0,1]
		// final clipping will keep only points inside (-1:1, -1:1, 0:1)
		PMatrix3D S=new PMatrix3D(
				pcanvas.width/2, 0, 0, pcanvas.width/2,
				0,-pcanvas.height/2, 0, pcanvas.height/2,
				0, 0, 0.5f, 0.5f,
				0, 0, 0, 1);
		PMatrix3D SInv=new PMatrix3D(S);
		SInv.invert();
		PMatrix3D relative=new PMatrix3D(fulltransform);
		if (debug>4)
			matprint("screenToRelative: SInv",SInv);
		relative.preApply(SInv);
		return relative;
	}

	private void matprint(String label, final PMatrix3D mat) {
		PApplet.println("P"+id+" "+label+":");
		mat.print();
	}

	@Deprecated
	public void decompose(final PMatrix3D projmodelview, final PMatrix3D model, PMatrix3D proj, PMatrix3D camera, boolean zknown) {
		// Decompose a complete mapping from world to screen coordinates and model into separate projection, camera, model, screen normalization
		// PMV = P*C*M
		// Approach: Assume P based on known projector lenses, set MV to identity;  multiply by inverses to find C
		// Can then decompose C into eye, aim, up
		// If zknown is false, then the 3rd row and 3rd column of projmodelview is unknown and needs to be inferred from camera matrix constraints
		proj=params.getProjection();
		matprint("decompose: proj",proj);

		PMatrix3D projinv=new PMatrix3D(proj);
		projinv.invert();
		matprint("decompose: projinv",projinv);

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
			camera.set(pv);
			camera.preApply(projinv);
			matprint("decompose: camera",camera);

			// Figure out what camera.m*2 should be to normalize the rows
			float norm0=camera.m00*camera.m00+camera.m01*camera.m01;
			float cm02=(norm0<1)?(float)Math.sqrt(1-norm0):0;   // Two possible solutions
			if (cm02!=0)
				PApplet.println("Two solution for cm02: +/-"+cm02);
			float norm1=camera.m10*camera.m10+camera.m11*camera.m11;
			float cm12=(norm1<1)?(float)Math.sqrt(1-norm1):0;  // Two possible solutions
			if (cm12!=0)
				PApplet.println("Two solution for cm12: +/-"+cm12);
			float norm2=camera.m20*camera.m20+camera.m21*camera.m21;
			float cm22=(norm2<1)?(float)Math.sqrt(1-norm2):0;  // TODO: Use this to resolve multiple solutions?
			PMatrix3D pvbest=pv;
			float bestmx=1e10f;
			for (int i=0;i<4;i++) {
				// Test the 4 possible signs of cm02 and cm12
				pv.m02=cm02/projinv.m00;
				PApplet.println("norm0="+norm0+", cm02="+cm02+", cm12="+cm12+", pv.m02="+pv.m02);
				pv.m12=(cm12-projinv.m13*pv.m32)/projinv.m11;
				matprint("reconstructed pv",pv);	
				PMatrix3D pvmtest=new PMatrix3D(pv);
				pvmtest.apply(model);
				float mx=maxdiff(pvmtest,projmodelview);
				PApplet.println("max error="+mx);
				if (mx<bestmx) {
					bestmx=mx;
					pvbest=new PMatrix3D(pv);
				}
				cm02=-cm02;
				if (i==1)
					cm12=-cm12;
			}
			pv=pvbest;

		}
		camera.set(pv);
		camera.preApply(projinv);

		matprint("decompose: camera",camera);

		decomposeCamera(camera);

		// Verify product (P*C*MV) == projmodelview
		PMatrix3D product=new PMatrix3D(proj);
		product.apply(camera);
		product.apply(model);

		matprint("deccompose: product",product);

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

	public void setProjection(PMatrix2D projection) {
		if (debug>0)
			PApplet.println("setProjection: "+projection.toString());
		params.setAbsProjection(projection);
	}

	public void setCameraView(PMatrix3D view) {
		camview=view;
		if (debug>0)
			matprint("setCameraView: view",view);
		if (debug>3) {
			matprint("setCameraView: pcanvas.modelview",pcanvas.modelview);
			PMatrix3D test=new PMatrix3D(view);
			test.preApply(params.getProjection());
			matprint("setCameraView:Proj*CV",test);
			matprint("setCameraView: pcanvas.projmodelview",pcanvas.projmodelview);
		}
		decomposeCamera(camview);
	}


	public void setWorld2Screen(PMatrix3D projmodelview) {
		// Setup canvas so that drawing commands follow the exact mapping 'projmodelview'
		// This needs to be made up from the composition of P*C*M
		// We can choose M to be the identity, so only need to break into P and C
		// Do that by using the known projector perspective mapping (from params.getProjection()) and compute C

		// PGraphics3D does additional scaling/centering of x,y after all the matrix transformations to get raw pixels
		// so we need to back that out of the target matrix
		w2s=projmodelview;
		PMatrix3D target=screenToRelative(projmodelview);
		if (debug>0)
			matprint("setWorld2Screen: projmodelview",projmodelview);

		// Use last received camview to set the unspecified parts of target since it doesn't map z-values
		PMatrix3D pcamview=new PMatrix3D(camview);
		pcamview.preApply(params.getProjection());
		if (debug>1)
			matprint("setWorld2Screen: pcamview",pcamview);

		//		target.m02=pcamview.m02;
		//		target.m12=pcamview.m12;
		//		target.m22=pcamview.m22;
		//		target.m32=pcamview.m32;
		//		target.m20=pcamview.m20;
		//		target.m21=pcamview.m21;
		//		target.m23=pcamview.m23;
		if (debug>1)
			matprint("setWorld2Screen: target",target);

		// New target has the desired value for canvas.projmodelview 
		// Decompose it into P*C
		PMatrix3D newcam=new PMatrix3D(target);
		PMatrix3D projInv = new PMatrix3D(params.getProjection()); projInv.invert();
		newcam.preApply(projInv);

		if (debug>1)
			matprint("setWorld2Screen: Extracted new camera matrix",newcam);

		pcanvas.setProjection(params.getProjection());
		pcanvas.beginCamera();
		pcanvas.resetMatrix();
		pcanvas.applyMatrix(newcam);
		pcanvas.endCamera();

		if (debug>1) {
			matprint("setWorld2Screen: projmodelview Matrix after applying transform",pcanvas.projmodelview);
			ttest(0,0,0);
			ttest(0,2,0);
			ttest(-2,2,0);
			ttest(2,2,0);
			ttest(0,0,1);
		}

	}
}
