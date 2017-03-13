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
		dbg("findCorner",0) << "Inconsistent corner point: " << Point(cx,cy) << " vs. " << Point(cx,cy2) << " with error " << std::abs(cy-cy2) << std::endl;
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
	static const float SEPFACTOR=3.7;		// Points separated by more this times the scan point separation are distinct objects
	static const int MINTARGETHITS=6;	// Minimum number of hits for it to be a target
	static const float MINTARGETWIDTH=0.15;	// Minimum width of target in meters
	static const float MAXTARGETWIDTH=0.35;	// Maximum width of target in meters
	static const float MAXFITERROR=0.05;	// RMS error between fitted corner and points
	static const float MAXCORNERERROR=20; // Error in angle of corner in degrees
	static const float MAXORIENTERROR=30;	// Error in which way corner is aiming in degrees
	static const float MAXTARGETDIST=8;		// Maximum distance
	static const float MINTARGETDIST=1;		// Minimum distance
	static const float MINSIDELENGTH=0.15;

	float dTheta=0;
	for (int i=0;i<background.size()-1;i++) {
	    // find a non-zero pair of ranges
	    if (background[i].getRange()>0 && background[i+1].getRange()>0) {
		dTheta=background[i+1].getTheta()-background[i].getTheta();
		break;
	    }
	}
	if (dTheta==0)
	    dbg("findTargetrs",0) << "all ranges are zero!" << std::endl;
	dbg("findTargets",4) << "dTheta=" << dTheta << std::endl;
	float lastRange=background[0].norm();
	int inTargetCnt=0;
	std::vector<Point> calCorners;		// Corners of possible alignment targets
	//dbgn("findTargets",4) << "back=[";
	//for (int ii=0;ii<background.size();ii++)
	//dbgn("findTargets",4) << background[ii].X() << "," << background[ii].Y() << ";";
	//dbgn("findTargets",4) << "]; plot(back(:,1),back(:,2),'o-');" << std::endl;

	for (int i=0;i<background.size();i++) {
	    float range=background[i].norm();
	    dbg("findTargets",(i%100==0)?4:10) <<  "i=" << i << ", range=" << range << ", inTargetCnt=" << inTargetCnt << std::endl;
	    float maxsep=std::min(range,lastRange)*dTheta*SEPFACTOR;	// Maximum distance between points of same object
	    dbg("findTargets",5) << "maxsep=" << maxsep << std::endl;
	    if (range==0 || fabs(range-lastRange)>maxsep)  {
		// At end of an object
		if (inTargetCnt>=MINTARGETHITS) {
		    // Has enough points
		    std::vector<Point> tgt(background.begin()+i-inTargetCnt+1,background.begin()+i-1);  // Omit first and last point of object
		    // Check for dimensions
		    float sz=(tgt.back()-tgt.front()).norm();
		    
		    if (sz>=MINTARGETWIDTH && sz<=MAXTARGETWIDTH) {
			// Decompose into a pair of lines at right angles
			float rms, cornerAngle,orient,dist;
			std::vector<Point> corners=findCorner(tgt,rms,cornerAngle,orient);
			dist=corners[1].norm();
			float side1= (corners[1]-corners[0]).norm();
			float side2= (corners[2]-corners[1]).norm();
			
			if (rms<=MAXFITERROR && fabs(cornerAngle-90)<MAXCORNERERROR && fabs(orient)<MAXORIENTERROR 
			    && dist<=MAXTARGETDIST && dist >= MINTARGETDIST && fmin(side1,side2)>=MINSIDELENGTH) {
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
			    calCorners.push_back(corners[1]);
			} else {
			    dbg("findTargets",4) << "alignment pattern rejected at scans " << i-inTargetCnt << "-" << i-1 << " at {" << corners[0] << ", " <<  corners[1] << ", " << corners[2] << "} with edge lengths  " 
					     << side1 << ", " << side2 << ", angle=" << cornerAngle << ", orient=" << orient 
					     << ", RMS=" << rms << ", dist=" << dist << std::endl;
			    }
		    } else {
			dbg("findTargets",4) << "ignoring target with size " << sz << " at scans " <<  i-inTargetCnt << "-" << i-1 << std::endl;
		    }
		}  else if (inTargetCnt>2) {
		    dbg("findTargets",5) << "ignoring target with too few hits at scans " << i-inTargetCnt << "-" << i-1 << std::endl;
		}
		inTargetCnt=1;  // Reset, since this is a different range
	    } else {
		inTargetCnt++;
	    }
	    lastRange=range;
	}
	return calCorners;
}
