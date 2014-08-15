#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>

#include "etherdream_bst.h"
#include "laser.h"
#include "dbg.h"
#include "point.h"
#include "drawing.h"
#include "ranges.h"

const int Laser::MAXDEVICEVALUE=32767;
const int Laser::MINDEVICEVALUE=-32768;

static const int MAXSLEWDISTANCE=65535/20;
// slewing is relative to mirror speed, but we need to figure out blanking in floor space (meters) coordinate space, so do rough conversion
static const float MEANTARGETDIST=4.0;   // Adjust for 4m
static const float FOV=M_PI/2;
static const float MAXSLEWMETERS=MAXSLEWDISTANCE/65535.0*MEANTARGETDIST*FOV;
static const int MINPOINTS=400;   // Minimum points per frame to avoid having laser focussed too tightly (brightly)

// Etherdream ID's in the order of units
static const unsigned long etherdreamIDS[]={0xe87f2f, 0xe74a90, 0xe7a2b2, 0xe7c3b0};
static const int MAXUNIT=sizeof(etherdreamIDS)/sizeof(etherdreamIDS[0]);

Laser::Laser(int _unit): labelColor(0,0,0),maxColor(0,1,0) {
    unit=_unit;
    PPS=40000;
    npoints=4000;
    blankingSkew=0; //3;
    targetSegmentLen=0.01f;
    preBlanks=3;
    postBlanks=16;
    labelColor=Color::getBasicColor(unit);
    d=NULL;
    enable(true);
    dbg("Laser.Laser",1) << "Maximum slew = " << MAXSLEWDISTANCE << " laser coords, " << MAXSLEWMETERS << " meters." << std::endl;
}

int Laser::open() {
    if (unit<0 || unit>=MAXUNIT)  {
	std::cerr << "Bad laser unit -- must be between 0 and " << MAXUNIT << std::endl;
	exit(0);
    }

    /* Sleep for a bit over a second, to ensure that we see broadcasts
     * from all available DACs. */
    usleep(1200000);

    int cc = etherdream_dac_count();
    if (!cc) {
	printf("No DACs found.\n");
	return -1;
    }

    printf("Looking for unit %d: 0x%lx:\n", unit, etherdreamIDS[unit]);
    int myed=-1;
    for (int i = 0; i < cc; i++) {
	unsigned long int eid=etherdream_get_id(etherdream_get(i));
	printf("%d: Ether Dream %06lx\n", i,eid);
	if (eid==etherdreamIDS[unit])
	    myed=i;
    }
    if (myed==-1) {
	std::cerr << "Etherdream unit " << unit << "(" << std::hex << etherdreamIDS[unit] << std::dec << " ) not found -- is it connected and on?" << std::endl;
	return  -1;
    }
    d = etherdream_get(myed);
    printf("Connecting to laser %d (0x%lx)...\n",unit,etherdreamIDS[unit]);
    if (etherdream_connect(d) < 0)
	return -1;
    return 0;
}

void Laser::update() {
    if (pts.size() < 2)  {
	dbg("Laser.update",1) << "Laser::update: not enough points (" << pts.size() << ") -- not updating" << std::endl;
	return;
    }

    if (d==0) {
	dbg("Laser.update",1) << "Laser not open" << std::endl;
	return;
    }
    dbg("Laser.update",1) << "Wait for ready." << std::endl;
    etherdream_wait_for_ready(d);
    dbg("Laser.update",1) << "Sending " << pts.size() << " points at " << PPS << " pps"  << std::endl;
    int res = etherdream_write(d,pts.data(), pts.size(), PPS, -1);
    dbg("Laser.update",1) << "Finished writing to etherdream, res=" << res << std::endl;
    if (res != 0)
	printf("write %d\n", res);
}

// Get specifc number of blanks at pos
std::vector<etherdream_point> Laser::getBlanks(int nblanks, etherdream_point pos) {
    std::vector<etherdream_point>  result;
    etherdream_point blank=pos;
    blank.r=0; blank.g=0;blank.b=0;
    for (int i=0;i<nblanks;i++) 
	result.push_back(blank);
    return result;
}

// Get number of needed blanks for given slew
std::vector<etherdream_point> Laser::getBlanks(etherdream_point initial, etherdream_point final) {
    static const int MINBLANKDIST=50;    // Minimum distance to bother with blanking (maps to about 1cm at 80deg FOV, 10m distance)
    std::vector<etherdream_point>  result;
    if (initial.r==0 &&initial.g==0 && initial.b==0 && final.r==0 &&final.g==0 && final.b==0) {
	dbg("Laser.getBlanks",2) << "No blanks needed; initial and final are already blanked" << std::endl;
	return result;
    }
    // Calculate distance in device coords
    int devdist=std::max(abs(initial.x-final.x),abs(initial.y-final.y));
    if (devdist>MINBLANKDIST) {
	int slewblanks=std::ceil(devdist/MAXSLEWDISTANCE);
	int nblanks=slewblanks+postBlanks;
	dbg("Laser.getBlanks",2) << "Inserting " << preBlanks << "(pre) + " << slewblanks << "(slew) + " << postBlanks << "(post) for a slew of distance " << devdist << " from " << initial.x << "," << initial.y << " to " << final.x << "," << final.y << std::endl;
	result=getBlanks(preBlanks,initial);
	std::vector<etherdream_point> blanks = getBlanks(nblanks,final);
	result.insert(result.end(), blanks.begin(), blanks.end());
    }
    return result;
}

// Convert drawing into a set of etherdream points
// Takes into account transformation to make all lines uniform brightness (i.e. separation of points is constant in floor dimensions)
void Laser::render(const Drawing &drawing) {
    if (d!=0) {
	int fullness=etherdream_getfullness(d);
	dbg("Laser.render",2) << "Fullness=" << fullness << std::endl;
	if (fullness>1) {
	    dbg("Laser.render",2) << "Etherdream already busy enough with " << fullness << " frames -- not rendering new drawing " << std::endl;
	    return;   
	}
    }
    if (showLaser && drawing.getNumElements()>0) {
	float drawLength=drawing.getLength();
	spacing=std::max(drawLength/npoints,targetSegmentLen);
	pts = transform.mapToDevice(drawing.getPoints(spacing).getPoints());
	//CPoints(pts).matlabDump("before prune");
	prune();
	//CPoints(pts).matlabDump("after prune");
	int nblanks=blanking();
	//CPoints(pts).matlabDump("after blanking");
	float effDrawLength=(pts.size()-nblanks)*spacing;
	dbg("Laser.render",2) << "Initial point count = " << pts.size() << " with " << nblanks << " blanks at a spacing of " << spacing << " for " << drawing.getNumElements() << " elements." << std::endl;
	dbg("Laser.render",2) << "Total drawing length =" << drawLength << ", drawable length=" << effDrawLength << std::endl;

	if (pts.size()>npoints &&  pts.size() > nblanks+2) {
	    spacing=std::max(effDrawLength/(npoints-nblanks),targetSegmentLen);
	    //drawing.getPoints(spacing).matlabDump("render");
	    pts = transform.mapToDevice(drawing.getPoints(spacing).getPoints());
	    prune();
	    nblanks=blanking();
	    dbg("Laser.render",2) << "Revised point count = " << pts.size() << " with " << nblanks << " blanks at a spacing of " << spacing << " for " << drawing.getNumElements() << " elements." << std::endl;
	}
    } else {
	pts.resize(0);
	dbg("Laser.render",2) << "Laser is not being shown" << std::endl;
    }

    if (pts.size() < MINPOINTS) {
	// Add some blanking on the end to fill it to desired number of points
	int nblanks=MINPOINTS-pts.size();
	dbg("Laser.render",2) << "Inserting " << nblanks << " blanks to pad out frame" << std::endl;
	etherdream_point pos;
	if (pts.size()>0)
	    pos=pts.back();
	else {
	    pos.x=0;pos.y=0;
	}
	std::vector<etherdream_point> blanks = getBlanks(nblanks,pos);
	pts.insert(pts.end(), blanks.begin(), blanks.end());
    }
    update();
}

void Laser::dumpPoints() const {
    std::string fname="ptdump-"+std::to_string(unit)+".txt";
    std::ofstream fd(fname);
    dbg("Laser.dumpPoints",1) << "Dumping " << pts.size() << " points to " << fname << std::endl;
    for (int i=0;i<pts.size();i++) {
	fd << pts[i].x << " " << pts[i].y << " " << (pts[i].g>0.5?1:0) << std::endl;
    }
}

// Prune a sequence of points by removing any segments that go out of bounds
void Laser::prune() {
    std::vector<etherdream_point> result;
    int pendingBlank=-1;
    for (unsigned int i=0;i<pts.size();i++) {
	bool isBlank=pts[i].r==0 && pts[i].g==0 &&pts[i].b==0;
	if (isBlank || !transform.onScreen(pts[i])) 
	    pendingBlank=i;
	else {
	    // In bounds
	    if (pendingBlank >= 0) {
		// Just came in bounds or visible, insert the last blank seen
		etherdream_point ptmp=pts[pendingBlank];
		ptmp.r=0;ptmp.g=0;ptmp.b=0;   // In case it was a non-blank, out of bounds point
		result.push_back(ptmp);
	    }
	    result.push_back(pts[i]);
	    pendingBlank=-1;
	}
    }
    // Don't need to put in trailing blanks
    dbg("Laser.prune",2) << "Pruned points from " << pts.size() << " to " << result.size() << std::endl;
    pts=result;
}

int Laser::blanking() {
    if (pts.size()<2)
	return 0;
    std::vector<etherdream_point> result;
    // Always put enough blanking at beginning and end to jump to (0,0)
    // This will allow a clean move when switching to an etherdream frame that has a different start/end from current frame
    etherdream_point homepos;
    homepos.x=0;homepos.y=0;homepos.g=65535;
    etherdream_point prevpos=homepos;
    int nblanks=0;
    for (unsigned int i=0;i<pts.size();i++) {
	if (pts[i].r>0 || pts[i].g>0 || pts[i].b>0) {
	    result.push_back(pts[i]);
	    prevpos=pts[i];
	} else {
	    // Insert blanks from prevpoint to here
	    std::vector<etherdream_point> blanks = getBlanks(prevpos,pts[i]);
	    if (blanks.size() > 0) {
		dbg("Laser.blanking",3) << "Adding " << blanks.size() << " blanks at position " << i << std::endl;
		result.insert(result.end(), blanks.begin(), blanks.end());
		nblanks+=blanks.size();
		prevpos=pts[i];
	    } else  {
		dbg("Laser.blanking",3) << "No blanks needed at position " << i << std::endl;
	    }
	}
    }
    if (result.size()>0) {
	// Insert blanks from lastpoint to home
	std::vector<etherdream_point> blanks = getBlanks(prevpos,homepos);
	dbg("Laser.blanking",3) << "Adding " << blanks.size() << " blanks at end " << std::endl;
	result.insert(result.end(), blanks.begin(), blanks.end());
	nblanks+=blanks.size();
    }
    dbg("Laser.blanking",2) << "Blanking increased points from " << pts.size() << " to " << result.size() << std::endl;
    pts=result;
    // Apply skew
    if (blankingSkew!=0) {
	for (int i=0;i<pts.size();i++) {
	    int newind=(i+blankingSkew)%pts.size();
	    result[newind].r=pts[i].r;
	    result[newind].g=pts[i].g;
	    result[newind].b=pts[i].b;
	}
	pts=result;
    }
    return nblanks;
}

void Laser::showTest() {
    etherdream_point pt;
    pts.clear();
    pt.r=0;pt.g=0;pt.r=0;
    static const int ngrid=7;
    static const int npoints=10000;
    static const int step=(MAXDEVICEVALUE-MINDEVICEVALUE)*ngrid*2/npoints;
    dbg("Laser.showTest",1) << "Showing laser test pattern with step size of " << step << std::endl;
    // Start and finish each scan at full range, but scan based on visible range stored in transform
    pt.g=65535;
    pt.x=MINDEVICEVALUE; pt.y=MINDEVICEVALUE;
    for (int i=0;i<ngrid;i++) {
      pt.y=(MAXDEVICEVALUE-MINDEVICEVALUE)*i/(ngrid-1)+MINDEVICEVALUE;
      if (pt.x<0) {
	for (int x=MINDEVICEVALUE;x<MAXDEVICEVALUE;x+=step) {
	  pt.x=x;
	  pts.push_back(pt);
	}
      } else {
	for (int x=MAXDEVICEVALUE;x>MINDEVICEVALUE;x-=step)  {
	  pt.x=x;
	  pts.push_back(pt);
	}
      }
    }
    // Now we should be near (range,range) assuming ngrid is odd
    for (int i=0;i<ngrid;i++) {
	pt.x=(MAXDEVICEVALUE-MINDEVICEVALUE)*(ngrid-1-i)/(ngrid-1)+MINDEVICEVALUE;
	if (pt.y<0) {
	    for (int y=MINDEVICEVALUE;y<MAXDEVICEVALUE;y+=step)  {
		pt.y=y;
		pts.push_back(pt);
	    }
	} else {
	    for (int y=MAXDEVICEVALUE;y>MINDEVICEVALUE;y-=step)  {
		pt.y=y;
		pts.push_back(pt);
	    }
	}
    }
    update();
}


void Laser::showOutline(const Bounds &b) {
    Transform &t=getTransform();

    if (!b.contains(t.flatToWorld(Point(0,0)))) {
	dbg("Lasers.showOutline",1) << "Laser not aiming at active area (aimed at " << t.flatToWorld(Point(0,0)) << "), not drawing bounds" << std::endl;
	return;
    }

    const Color outlineColor=Color(0.0,1.0,0.0);
    const bool drawDiagonals=false;

    pts.clear();
    // Need to be inside maximums by at least one pixel or pruning will think its saturated and take it out

    Point p;
    static const int nsegments=200;
    std::vector<Point> flatPts;
    p.setX(t.getMinX());
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+i*(t.getMaxY()-t.getMinY())/(nsegments-1));
	flatPts.push_back(p);
    }
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+i*(t.getMaxX()-t.getMinX())/(nsegments-1));
	flatPts.push_back(p);
    }
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	flatPts.push_back(p);
    }
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+(nsegments-i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	flatPts.push_back(p);
    }
    if (drawDiagonals) {
	// Diagonal
	for (int i=0;i<nsegments;i++) {
	    p.setX(t.getMinX()+(i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	    p.setY(p.X());
	    flatPts.push_back(p);
	}
	// Return to a good position
	for (int i=0;i<nsegments;i++) {
	    p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	    flatPts.push_back(p);
	}
	// Diagonal
	for (int i=0;i<nsegments;i++) {
	    p.setX(t.getMinX()+(nsegments-i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	    p.setY(-p.X());
	    flatPts.push_back(p);
	}
	// Return to a good position
	for (int i=0;i<nsegments;i++) {
	    p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	    flatPts.push_back(p);
	}
    }
    dbg("Lasers.showOutline",3) << "outline (flat,dev)=" << std::endl;
    for (int j=0;j<flatPts.size();j++) {
	Point world=t.flatToWorld(flatPts[j]*0.99f);	// Inset slightly
	if (!b.contains(world)) {
	    Point good=Point(0,0);  // Had checked above that 0,0 is good!
	    Point bad=flatPts[j];
	    while ((bad-good).norm() > .001) {
	       flatPts[j]=(good+bad)/2;
	       world=t.flatToWorld(flatPts[j]*0.99f);
	       if (b.contains(world))
		    good=flatPts[j];
		else
		    bad=flatPts[j];
	    }
	}
	Point constrainWorld=b.constrainPoint(world);
	Point devPoint=t.mapToDevice(constrainWorld);
	pts.push_back(t.cPointToEtherdream(CPoint(devPoint,outlineColor)));
	dbgn("Lasers.showOutline",3) << flatPts[j].X() << ", " << flatPts[j].Y() << ", " << world.X() << "," << world.Y() << "," << constrainWorld.X() << "," << constrainWorld.Y() << "," << devPoint.X() << "," << devPoint.Y() << "," << pts[j].x << "," << pts[j].y << std::endl;
    }
    prune();
    dbg("Lasers.showOutline",3) << "pts after prune=" << std::endl;
    for (int i=0;i<pts.size();i++)
	dbgn("Lasers.showOutline",1) << pts[i].x << "," <<pts[i].y << "," <<pts[i].r << "," <<pts[i].g << "," <<pts[i].b << std::endl;
    update();
}

// Determine fraction of the object (in floor coordinates) is in the field of view of the given laser
float Laser::fracOnScreen(const CPoints &cpts) const {
    int viscnt=0;
    const std::vector<CPoint> &pts=cpts.getPoints();
    for (int i=0;i<pts.size();i++) {
	Point devPt=transform.mapToDevice((Point)pts[i]);
	if (transform.onScreen(devPt))
	    viscnt++;
    }
    dbg("Laser.fracOnScreen",3) << viscnt << "/" << pts.size() << " point on screen" << std::endl;
    return viscnt*1.0f/pts.size();
}

// Determine fraction of points (in floor coordinates) whose view from origin is obstructed by other objects
// Only objects that are more than originGap from origin and more than targetGap from the point of interest are considered blockages
float Laser::fracShadowed(const CPoints &cpts, const Ranges &ranges, float originGap, float targetGap) const {
    static const int DOWNSAMPLE=4;
    Point origin=transform.getOrigin();
    int shadowCnt=0;
    const std::vector<CPoint> &pts=cpts.getPoints();
    for (int i=0;i<pts.size();i+=DOWNSAMPLE) {
	if (ranges.isObstructed(origin,pts[i],originGap,targetGap))
	    shadowCnt++;
    }
    dbg("Laser.fracShadowed",3) << "View from " << origin << " shadowed for " << shadowCnt*DOWNSAMPLE << "/" << pts.size() << " points" << std::endl;
    return shadowCnt*1.0f/pts.size()*DOWNSAMPLE;
}

// Remove shadowed points from a set of CPoints
CPoints Laser::removeShadowed(const CPoints &cpts, const Ranges &ranges, float originGap, float targetGap) const {
    CPoints result;
    Point origin=transform.getOrigin();
    const std::vector<CPoint> &pts=cpts.getPoints();
    bool deleting=false;
    for (int i=0;i<pts.size();i++) {
	if (pts[i].getColor()==Color(0,0,0)) {
	    // Blank
	    result.push_back(pts[i]);
	    deleting=false;  // No longer need to insert a blank when resuming
	} else if (!ranges.isObstructed(origin,pts[i],originGap,targetGap)) {
	    // Unobstructed
	    if (deleting) {
		// Add a soft blank after a deleted group
		result.push_back(CPoint((Point)pts[i],Color(0,0,0)));
		deleting=false;
	    }
	    result.push_back(pts[i]);
	} else {
	    // skip point
	    deleting=true;
	}
    }
    if (result.size()>0 && result.front().getColor()!=Color(0,0,0)) {
	// Need a blank at the beginning
	dbg("Laser.removeShadowed",0) << "No blank at beginning of " << result.size() << " points" << std::endl;
    }
    dbg("Laser.removeShadowed",3) << "View from " << origin << " reduced point count from " << cpts.size() << " to " << result.size() << std::endl;
    return result;
}

// Compute mean distance of object from given point (typcial from laser origin or center of project)
float Laser::meanDistance(const CPoints &cpts) const {
    Point origin=transform.getOrigin();
    float d=0;
    const std::vector<CPoint> &pts=cpts.getPoints();
    for (int i=0;i<pts.size();i++) {
	d+=(origin-pts[i]).norm();
    }
    return d/pts.size();
}


