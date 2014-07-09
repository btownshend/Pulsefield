#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>

#include "etherdream_bst.h"
#include "laser.h"
#include "dbg.h"
#include "point.h"
#include "drawing.h"

static const int MAXSLEWDISTANCE=65535/20;
// slewing is relative to mirror speed, but we need to figure out blanking in floor space (meters) coordinate space, so do rough conversion
static const float MEANTARGETDIST=4.0;   // Adjust for 4m
static const float FOV=M_PI/2;
static const float MAXSLEWMETERS=MAXSLEWDISTANCE/65535.0*MEANTARGETDIST*FOV;

Laser::Laser(int _unit): labelColor(0,0,0),maxColor(0,1,0) {
    unit=_unit;
    PPS=40000;
    npoints=1000;
    blankingSkew=3;
    preBlanks=2;
    postBlanks=16;
    labelColor=Color::getBasicColor(unit);
    showLaser = true;
    dbg("Laser.Laser",1) << "Maximum slew = " << MAXSLEWDISTANCE << " laser coords, " << MAXSLEWMETERS << " meters." << std::endl;
}

int Laser::open() {
    etherdream_lib_start();

    /* Sleep for a bit over a second, to ensure that we see broadcasts
     * from all available DACs. */
    usleep(1200000);

    int cc = etherdream_dac_count();
    if (!cc) {
	printf("No DACs found.\n");
	return -1;
    }

    int i;
    for (i = 0; i < cc; i++) {
	printf("%d: Ether Dream %06lx\n", i,etherdream_get_id(etherdream_get(i)));
    }

    if (cc<=unit) {
	printf("Requested laser unit %d, but only have %d lasers\n", unit, cc);
	return -1;
    }
    // Kludge to make sure we get ordered id's (TODO: make general)
    if (cc==2 && etherdream_get_id(etherdream_get(0)) < etherdream_get_id(etherdream_get(1))) {
	std::cout << "Swapped etherdream order" << std::endl;
	d = etherdream_get(1-unit);    
    } else
	d = etherdream_get(unit);
    printf("Connecting to laser %d...\n",unit);
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
    std::vector<etherdream_point>  result;
    if (initial.r==0 &&initial.g==0 && initial.b==0 && final.r==0 &&final.g==0 && final.b==0) {
	dbg("Laser.getBlanks",2) << "No blanks needed; initial or final are already blanked" << std::endl;
	return result;
    }
    // Calculate distance in device coords
    int devdist=std::max(abs(initial.x-final.x),abs(initial.y-final.y));
    if (devdist>1) {
	int nblanks=std::ceil(devdist/MAXSLEWDISTANCE)+postBlanks;
	dbg("Laser.getBlanks",2) << "Inserting " << preBlanks << " + " << nblanks << " for a slew of distance " << devdist << " from " << initial.x << "," << initial.y << " to " << final.x << "," << final.y << std::endl;
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
	spacing=drawing.getLength()/npoints;
	pts = transform.mapToDevice(drawing.getPoints(spacing));
	prune();
	int nblanks=blanking();
	dbg("Laser.render",2) << "Initial point count = " << pts.size() << " with " << nblanks << " blanks, compared to planned " << npoints << " for " << drawing.getNumElements() << " elements." << std::endl;
	int redonpoints=npoints;
	if (nblanks> redonpoints/2)  {
	    redonpoints=nblanks*2;
	    dbg("Laser.render",2) << "More than half of points are blanks, increasing target to " << redonpoints << std::endl;
	}
	    
	if (std::abs(redonpoints-(int)pts.size()) > 10 &&  pts.size() > nblanks+2) {
	    float scaleFactor=(redonpoints-nblanks)*1.0/(pts.size()-nblanks);
	    dbg("Laser.render",2) << "Have " << nblanks << " blanks; adjusting spacing by a factor of " << scaleFactor << std::endl;
	    spacing/=scaleFactor;
	    pts=transform.mapToDevice(drawing.getPoints(spacing));
	    prune();
	    (void)blanking();
	    dbg("Laser.render",2) << "Revised point count = " << pts.size() << " compared to planned " << redonpoints << " for " << drawing.getNumElements() << " elements." << std::endl;
	}
	dbg("Laser.render",2) << "Rendered drawing into " << pts.size() << " points with a spacing of " << spacing << std::endl;
    } else {
	pts.resize(0);
	dbg("Laser.render",2) << "Laser is not being shown" << std::endl;
    }

    if (pts.size() < npoints) {
	// Add some blanking on the end to fill it to desired number of points
	int nblanks=npoints-pts.size();
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
	if (pts[i].x != -32768  && pts[i].x != 32767 && pts[i].y != -32768 && pts[i].y != 32767)  {
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
    etherdream_point prevpos=pts.back();
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
    if (jumped && result.size()>0) {
	// Insert blanks from prevpoint to here
	std::vector<etherdream_point> blanks = getBlanks(prevpos,result.front());
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
    static const int ngrid=9;
    static const int blank=60;
    static const int npoints=10000;
    static const int fullrange=32767;
    static const int range=28000;
    static const int step=2*fullrange*ngrid*2/npoints;
    dbg("Laser.showTest",1) << "Showing laser test pattern with step size of " << step << std::endl;
    // Start and finish each scan near (-range,-range)
    pt.g=65535;
    pt.x=-fullrange; pt.y=-fullrange;
    for (int i=0;i<ngrid;i++) {
      pt.y=2*range*i/(ngrid-1)-range;
      if (pt.x<0) {
	for (int x=-fullrange;x<fullrange;x+=step) {
	  pt.x=x;
	  pts.push_back(pt);
	}
      } else {
	for (int x=fullrange;x>-fullrange;x-=step)  {
	  pt.x=x;
	  pts.push_back(pt);
	}
      }
    }
    // Now we should be near (range,range) assuming ngrid is odd
    for (int i=0;i<ngrid;i++) {
      pt.x=2*range*(ngrid-1-i)/(ngrid-1)-range;
      if (pt.y<0) {
	for (int y=-fullrange;y<fullrange;y+=step)  {
	  pt.y=y;
	  pts.push_back(pt);
	}
      } else {
	for (int y=fullrange;y>-fullrange;y-=step)  {
	  pt.y=y;
	  pts.push_back(pt);
	}
      }
    }
    update();
}

