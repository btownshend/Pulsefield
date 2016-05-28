import processing.core.PApplet;
import processing.core.PMatrix3D;

public class ProjectorParameters {
	// Parameters defining a project]
	String name;
	float distance;   // Distance from screen used for parameterization
	float voffset;    // Distance projector is below bottom of screen
	float screenheight;     // Image height
	int height, width;   // Resolution in pixels
	float left, right, top, bottom, near, far;
	
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
	}
	
	static public ProjectorParameters eh320ust() {
		// Optoma EH320UST - from manual page 22
		return new ProjectorParameters("EH320UST",0.56f,0.224f,1.25f,1920,1080);
	}
	
	public PMatrix3D getProjection() {
		float A=(right+left)/(right-left);
		float B=(top+bottom)/(top-bottom);
		float C=-(far+near)/(far-near);
		float D=-2*far*near/(far-near);
		return new PMatrix3D(
				2*near/(right-left),0,                  A,0,
				0,                  -2*near/(top-bottom),B,0,  // proj(2,2) is negated from expected (otherwises doesn't match observed matrix after calling frustum()
				0,                  0,                  C,D,
				0,                  0,                 -1,0);
	}
}
