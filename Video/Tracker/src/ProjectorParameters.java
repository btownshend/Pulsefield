import processing.core.PApplet;
import processing.core.PMatrix2D;
import processing.core.PMatrix3D;
import processing.core.PVector;

public class ProjectorParameters {
	// Parameters defining a project]
	String name;
	float distance;   // Distance from screen used for parameterization
	float voffset;    // Distance projector is below bottom of screen
	float screenheight;     // Image height
	int height, width;   // Resolution in pixels
	float left, right, top, bottom, near, far;
	PMatrix3D projMatrix;  // 4x4 projection matrix  (applies perspective to scene in camera view; maps to cube(-1:1, -1:1, -1:1) ) 
	
	public ProjectorParameters(String name, float distance, float voffset, float screenheight, int width, int height) {
		this.name=name;
		this.distance=distance;
		this.voffset=voffset;
		this.screenheight=screenheight;
		this.height=height;
		this.width=width;
		computeProjection();
	}
	
	private void computeProjection() {
		// Compute projection values (left, right, top, bottom, near, far) that can be used with frustum()
		near=1f;   // Arbitrary location of near frustum (in meters)
		far=50f;   // Far enough to ensure nothing on floor is clipped
		float s=near/distance;   // Scaling of parameters
		bottom=s*voffset;
		top=bottom+s*screenheight;
		left=-(top-bottom)*width/height/2;
		right=-left;
		PApplet.println(name+": left="+left+", right="+right+", bottom="+bottom+", top="+top+", near="+near+", far="+far);
		float A=(right+left)/(right-left);
		float B=(top+bottom)/(top-bottom);
		float C=-(far+near)/(far-near);
		float D=-2*far*near/(far-near);
		// Set projection matrix based on above
		setProjection(new PMatrix3D(
				2*near/(right-left),0,                  A,0,
				0,                  -2*near/(top-bottom),B,0,  // proj(2,2) is negated from expected (otherwises doesn't match observed matrix after calling frustum()
				0,                  0,                  C,D,
				0,                  0,                 -1,0));
	}
	
	static public ProjectorParameters eh320ust() {
		// Optoma EH320UST - from manual page 22
		return new ProjectorParameters("EH320UST",0.56f,0.224f,1.25f,1920,1080);
	}
	
	public PMatrix3D getProjection() {
		return projMatrix;
	}

	private void testProjection(PVector in, PVector expect) {
		float result[]=new float[4];
		float invect[]={in.x,in.y,in.z,1.0f};
		projMatrix.mult(invect,result);
		PVector res=new PVector(result[0]/result[3],result[1]/result[3],result[2]/result[3]);
		PApplet.println("testProjection: proj*"+in+" -> "+res+", expected "+expect);
	}
	
	public void setProjection(PMatrix3D projection) {
		PApplet.println("ProjectorParameters.setProjection:");
		projection.print();
		projMatrix=projection;
		// Verify that it is reasonable
		testProjection(new PVector((left+right)/2,-(top+bottom)/2,-near),new PVector(0,0,-1));
		testProjection(new PVector((left+right)/2*far/near,-(top+bottom)/2*far/near,-far),new PVector(0,0,1));
		testProjection(new PVector(left,-bottom,-near),new PVector(-1,-1,-1));
		testProjection(new PVector(right,-top,-near),new PVector(1,1,-1));
	}
	
	public void setAbsProjection(PMatrix2D projection) {
		PApplet.println("ProjectorParameters.setAbsProjection:");
		projection.print();
		float C=-(far+near)/(far-near);
		float D=-2*far*near/(far-near);
		projMatrix.set(projection.m00*2/width,projection.m01*2/width,projection.m02*2/width-1,0,
				projection.m10*2/height,projection.m11*2/height,projection.m12*2/height-1,0,
				0,0,C,D,
				0,0,-1,0);
		PApplet.println("Equivalent relative 3d projection:");
		projMatrix.print();
		// Verify that it is reasonable
		testProjection(new PVector((left+right)/2,-(top+bottom)/2,-near),new PVector(0,0,-1));
		testProjection(new PVector((left+right)/2*far/near,-(top+bottom)/2*far/near,-far),new PVector(0,0,1));
		testProjection(new PVector(left,-bottom,-near),new PVector(-1,-1,-1));
		testProjection(new PVector(right,-top,-near),new PVector(1,1,-1));
	}
}
