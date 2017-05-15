#include <vector>
#include <cmath>
#include <assert.h>
#include "findtargets.h"
#include "dbg.h"

#ifdef DRAWING
#include "drawing.h"
#endif

static float orthoval(const std::vector<float> fit, float x) {
    return x*fit[0]+fit[1];
}

static std::vector<float> orthofit(const std::vector<Point> &pts,float &err) {
    assert(pts.size()>=2);
    Point sum(0,0);
    for (int i=0;i<pts.size();i++)
	sum=sum+pts[i];
    sum=sum/pts.size();
    double sxx=0,sxy=0,syy=0;
    for (int i=0;i<pts.size();i++) {
	sxx+=pow(pts[i].X()-sum.X(),2.0);
	sxy+=(pts[i].X()-sum.X())*(pts[i].Y()-sum.Y());
	syy+=pow(pts[i].Y()-sum.Y(),2.0);
    }
    sxx/=(pts.size()-1);
    sxy/=(pts.size()-1);
    syy/=(pts.size()-1);
    std::vector<float> fit(2);
    fit[0]=(syy-sxx+sqrt(pow(syy-sxx,2.0)+4*pow(sxy,2.0)))/(2*sxy);
    fit[1]=sum.Y()-fit[0]*sum.X();
    err=0;
    for (int i=0;i<pts.size();i++) {
	float y=orthoval(fit,pts[i].X());
	err+=pow(y-pts[i].Y(),2.0);
	dbg("orthofit",3) << "pt[" << i << "] = " << pts[i] << std::endl;
    }
    dbg("orthofit",3) << "sxx=" << sxx << ", sxy=" << sxy << ", syy=" << syy << std::endl;
    dbg("orthofit",3) << "fit=" << fit[0] << "*x + " << fit[1] << "; err^2=" << err << std::endl;
    return fit;
}

// Fit a set of points to a right angle corner
static std::vector<Point> findCorner(const std::vector<Point> &pts, float &rms, float &cornerAngle, float &orient)  {
    assert (pts.size() >= 4);
    std::vector<Point> soln(3);
    // Try each possible divistion into two edges with at least 2 pts/edbge
    float minerr=1e10;
    for (int i=1;i<pts.size()-2;i++) {
	// Edges are [0,i] and [i+1,end]
	float e1,e2;
	std::vector<float> fit1=orthofit(std::vector<Point>(&pts[0],&pts[i+1]),e1);
	std::vector<float> fit2=orthofit(std::vector<Point>(&pts[i+1],&pts[pts.size()]),e2);
	dbg("findCorner",3) << "Using " << (i+1) << "/" << (pts.size()-i-1) << " pts -> err=" << (e1+e2) << std::endl;
	if (e1+e2 < minerr) {
	    minerr=e1+e2;
	    float x=pts[0].X();
	    float y=orthoval(fit1,pts[0].X());
	    soln[0]=Point(x,y);
	    float cx=-(fit1[1]-fit2[1])/(fit1[0]-fit2[0]);
	    float cy=orthoval(fit1,cx);
	    float cy2=orthoval(fit2,cx);
	    if (std::abs(cy-cy2)>0.001) {
		dbg("findCorner",1) << "Inconsistent corner point: " << Point(cx,cy) << " vs. " << Point(cx,cy2) << " with error " << std::abs(cy-cy2) << std::endl;
	    }
	    soln[1]=Point(cx,cy);
	    x=pts[pts.size()-1].X();
	    y=orthoval(fit2,x);
	    soln[2]=Point(x,y);
	    rms=sqrt((e1+e2)/pts.size());
	    float a1=(soln[0]-soln[1]).getTheta()*180/M_PI;
	    float a2=(soln[2]-soln[1]).getTheta()*180/M_PI;
	    float a3=soln[1].getTheta()*180/M_PI;
	    cornerAngle=a2-a1; if (cornerAngle<0) cornerAngle+=360;
	    orient=a1+cornerAngle/2-a3;  if (orient>180) orient-=360;  if (orient<-180) orient+=360;
	}
    }
    return soln;
}

#ifdef DRAWING
std::vector<Point> findTargets( const std::vector<Point> background, Drawing *drawing) {
#else
std::vector<Point> findTargets( const std::vector<Point> background) {
#endif
	static const float MAXSEP=0.3;		// Ends of target must be clear of other objects for this distance (meters)
	static const int MINTARGETHITS=6;	// Minimum number of hits for it to be a target
	static const float MINTARGETWIDTH=0.335;	// Minimum width of target cross-section in meters
	static const float MAXTARGETWIDTH=0.553+.05;	// Maximum width of target in meters
	static const float MAXFITERROR=0.01;	// RMS error between fitted corner and points
	static const float CORNERANGLE=109.3;   // Angle of corner in degrees
	static const float MAXCORNERERROR=4; // Error in angle of corner in degrees
	static const float MAXORIENTERROR=180;	// Error in which way corner is aiming in degrees
	static const float MAXTARGETDIST=10;		// Maximum distance
	static const float MINTARGETDIST=0.2;		// Minimum distance
	static const float SIDELENGTH=0.315;
	static const float SIDEERROR=.05;
	static const float THICKNESS=0.0078;    // Distance from inner corner to outer corner

	float dTheta=0;
	for (int i=0;i<background.size()-1;i++) {
	    // find a non-zero pair of ranges
	    if (background[i].getRange()>0 && background[i+1].getRange()>0) {
		dTheta=background[i+1].getTheta()-background[i].getTheta();
		break;
	    }
	}
	if (dTheta==0)
	    dbg("findTargets",0) << "all ranges are zero!" << std::endl;
	dbg("findTargets",4) << "dTheta=" << dTheta << std::endl;
	int inTargetCnt=0;
	std::vector<Point> calCorners;		// Corners of possible alignment targets
	dbg("findTargets",5) << "back=" << background << std::endl;

	for (int i=0;i<background.size();i++) {
	    if (inTargetCnt==0) {
		// Check that this segment starts clear of any prior hits
		bool distinct=true;
		for (int j=i-1;j>=0;j--) {
			float d=(background[j]-background[i]).norm();
			if ( d < MAXSEP) {
			    dbg("findTargets",5) << "Found prior point " << j << " that is only " << d << " from point " << i << "; not beginning of distinct object" << std::endl;
			    distinct=false;
			    break;
			}
		    }
		if (!distinct)
		    continue;   // Advance to next point to try again
	    }
	    inTargetCnt++;
		
	    bool atEnd=true;
	    dbg("findTargets",5) << "Testing if potential hit at " << i-inTargetCnt+1 << ":" << i << " is clear of subsequent objects" << std::endl;
	    // Check if any subsequent points are close
	    for (int j=i+1;j<background.size();j++)
		if ( (background[j]-background[i]).norm() < MAXSEP) {
		    dbg("findTargets",5) << "Found point " << j << " that is only " << (background[j]-background[i]).norm() << " from point " << i << "; not end of object" << std::endl;
		    atEnd=false;
		    break;
	    }
	    if (atEnd) {
		// At end of an object consisting of inTargetCnt points (from point i-inTargetCnt+1 to i )
		if (inTargetCnt>=MINTARGETHITS) {
		    // Has enough points
		    std::vector<Point> tgt(background.begin()+i-inTargetCnt+2,background.begin()+i);  // Omit first and last point of object
		    // Check for dimensions
		    float sz=(tgt.back()-tgt.front()).norm();
		    
		    if (sz>=MINTARGETWIDTH && sz<=MAXTARGETWIDTH) {
			// Decompose into a pair of lines at right angles
			float rms, cornerAngle,orient,dist;
			std::vector<Point> corners=findCorner(tgt,rms,cornerAngle,orient);
			dist=corners[1].norm();
			float side1= (corners[1]-corners[0]).norm();
			float side2= (corners[2]-corners[1]).norm();
			dbg("findTargets",5) << "pts=" << tgt << std::endl;
			
			if (rms<=MAXFITERROR && ( fabs(cornerAngle-CORNERANGLE)<MAXCORNERERROR  || fabs(cornerAngle-(360-CORNERANGLE))<MAXCORNERERROR) && fabs(orient)<MAXORIENTERROR 
			    && dist<=MAXTARGETDIST && dist >= MINTARGETDIST && fabs(side1-SIDELENGTH)<=SIDEERROR && fabs(side2-SIDELENGTH)<=SIDEERROR) {
			    dbg("findTargets",3) << "alignment pattern accepted at scans " << i-inTargetCnt << "-" << i-1 << " at {" << corners[0] << ", " <<  corners[1] << ", " << corners[2] << "} with edge lengths  " 
						 << side1<< ", " << side2 << ", angle=" << cornerAngle << ", orient=" << orient 
						 << ", RMS=" << rms << ", dist=" << dist << std::endl;
			    // Draw the hits on the target as a polygon
#ifdef DRAWING
			    if (drawing)
				drawing->drawPolygon(corners,Color(0.0,1.0,0.0));
			    // Draw a circle around target
			    //			    globalDrawing.drawCircle(corners[1],0.02,bgColor);
#endif
			    if (cornerAngle>180) {
				Point offset=corners[1]/corners[1].norm()*THICKNESS;
				dbg("findTargets",3) << "Offsetting by " << offset << std::endl;
				calCorners.push_back(corners[1]+offset);
			    } else {
				calCorners.push_back(corners[1]);
			    }
			} else {
			    dbg("findTargets",4) << "alignment pattern rejected at scans " << i-inTargetCnt << "-" << i-1 << " at {" << corners[0] << ", " <<  corners[1] << ", " << corners[2] << "} with edge lengths  " 
					     << side1 << ", " << side2 << ", angle=" << cornerAngle << ", orient=" << orient 
					     << ", RMS=" << rms << ", dist=" << dist << std::endl;
			    }
		    } else {
			dbg("findTargets",4) << "ignoring target with size " << sz << " at scans " <<  i-inTargetCnt+1 << "-" << i << std::endl;
		    }
		}  else if (inTargetCnt>2) {
		    dbg("findTargets",5) << "ignoring target with too few hits at scans " << i-inTargetCnt+1 << "-" << i << std::endl;
		}
		inTargetCnt=0;  // Ready to start a new object
	    }
	}
	return calCorners;
}
