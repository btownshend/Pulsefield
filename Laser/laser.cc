#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>

#include "etherdream_bst.h"
#include "laser.h"
#include "dbg.h"
#include "point.h"
#include "drawing.h"

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
	dbg("Laser.getBlanks",2) << "No blanks needed; initial or final are already blanked" << std::endl;
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
	pts = transform.mapToDevice(drawing.getPoints(spacing));
	prune();
	int nblanks=blanking();
	float effDrawLength=(pts.size()-nblanks)*spacing;
	dbg("Laser.render",2) << "Initial point count = " << pts.size() << " with " << nblanks << " blanks at a spaing of " << spacing << " for " << drawing.getNumElements() << " elements." << std::endl;
	dbg("Laser.render",2) << "Total drawing length =" << drawLength << ", but effective length=" << effDrawLength << std::endl;

	if (drawLength!=effDrawLength &&  pts.size() > nblanks+2) {
	    spacing=std::max(effDrawLength/(npoints-nblanks),targetSegmentLen);
	    pts=transform.mapToDevice(drawing.getPoints(spacing));
	    prune();
	    nblanks=blanking();
	    dbg("Laser.render",2) << "Revised point count = " << pts.size() << " with " << nblanks << " blanks at a spaing of " << spacing << " for " << drawing.getNumElements() << " elements." << std::endl;
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
    bool oobs=false;
    for (unsigned int i=0;i<pts.size();i++) {
	if (transform.onScreen(pts[i])) {
	    // In bounds
	    if (oobs && result.size()>0) {
		// Just came in bounds, insert a blank (will be expanded by blanking)
		std::vector<etherdream_point> blanks = Laser::getBlanks(1,result.back());
		result.insert(result.end(), blanks.begin(), blanks.end());
	    }
	    result.push_back(pts[i]);
	    oobs=false;
	} else
	    oobs=true;
    }
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
    bool jumped=true;
    int nblanks=0;
    for (unsigned int i=0;i<pts.size();i++) {
	if (pts[i].r>0 || pts[i].g>0 || pts[i].b>0) {
	    if (jumped) {
		// Insert blanks from prevpoint to here
		std::vector<etherdream_point> blanks = getBlanks(prevpos,pts[i]);
		dbg("Laser.blanking",3) << "Adding " << blanks.size() << " blanks at position " << i << std::endl;
		result.insert(result.end(), blanks.begin(), blanks.end());
		nblanks+=blanks.size();
		jumped=false;
	    }
	    result.push_back(pts[i]);
	    prevpos=pts[i];
	} else {
	    // Blanking, mark 
	    jumped=true;
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


void Laser::showOutline() {
    const Color outlineColor=Color(0.0,1.0,0.0);

    pts.clear();
    // Need to be inside maximums by at least one pixel or pruning will think its saturated and take it out
    Transform &t=getTransform();

    CPoint p;
    p.setColor(outlineColor);
    static const int nsegments=200;
    p.setX(t.getMinX());
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+i*(t.getMaxY()-t.getMinY())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+i*(t.getMaxX()-t.getMinX())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+(nsegments-i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    // Diagonal
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+(i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	p.setY(p.X());
	pts.push_back(t.flatToDevice(p));
    }
    // Return to a good position
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    // Diagonal
    for (int i=0;i<nsegments;i++) {
	p.setX(t.getMinX()+(nsegments-i-1)*(t.getMaxX()-t.getMinX())/(nsegments-1));
	p.setY(-p.X());
	pts.push_back(t.flatToDevice(p));
    }
    // Return to a good position
    for (int i=0;i<nsegments;i++) {
	p.setY(t.getMinY()+(nsegments-i-1)*(t.getMaxY()-t.getMinY())/(nsegments-1));
	pts.push_back(t.flatToDevice(p));
    }
    dbg("Lasers.render",3) << "outline=";
    for (int j=0;j<pts.size();j++)
	dbgn("Lasers.render",3) << "(" << pts[j].x << "," << pts[j].y << ")  ";
    dbgn("Lasers.render",3) << std::endl;
    update();
}
