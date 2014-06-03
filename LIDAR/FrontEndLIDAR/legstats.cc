#include <iomanip>
#include <math.h>
#include "person.h"
#include "legstats.h"
#include "parameters.h"
#include "dbg.h"

LegStats::LegStats() {
    diam=INITLEGDIAM;
    diamSigma=LEGDIAMSIGMA;
    sep=MEANLEGSEP;
    sepSigma=LEGSEPSIGMA;
    leftness=0.0;
    facing=0.0;
    facingSEM=FACINGSEM;
}

std::ostream &operator<<(std::ostream &s, const LegStats &ls) {
    s << std::fixed << std::setprecision(0)
      << "diam:  " << ls.diam
      << ",sep: " << ls.sep
      << std::setprecision(3);
    return s;
}

void LegStats::update(const Person &p) {
    // Update leftness
    Point legdiff=p.getLeg(1).getPosition()-p.getLeg(0).getPosition();
    leftness=leftness*(1-1/LEFTNESSTC)+legdiff.dot(Point(-p.getVelocity().Y(),p.getVelocity().X()))/LEFTNESSTC;

    // Update facing
    float curfacing=legdiff.getTheta()+M_PI/2;
    if (leftness<0)
	curfacing-=M_PI;
    float diff=curfacing-facing;
    while (diff>M_PI)
	diff-=2*M_PI;
    while (diff<-M_PI)
	diff+=2*M_PI;
    dbg("LegStats",4) << "Update leftness of " << facing*180/M_PI << " towards " << curfacing*180/M_PI << " to " << (facing+diff/LEGSTATSTC)*180/M_PI << std::endl;
    facing += diff/LEGSTATSTC;
    while (facing>M_PI)
	facing-=2*M_PI;
    while (facing<-M_PI)
	facing+=2*M_PI;
    
    // Update separation
    if (p.getLeg(0).isVisible() && p.getLeg(1).isVisible()) {
	// Both legs visible, update separation estimate
	float cursep=(p.getLeg(0).getPosition()-p.getLeg(1).getPosition()).norm();
	sep = sep*(1-1/LEGSTATSTC) + cursep/LEGSTATSTC;
	// TODO: track sepSigma
	if (sep>MEANLEGSEP*2) {
	    dbg("LegStats",2) << "Leg separation too high at " << sep << "; reducing to " << MEANLEGSEP*2 << std::endl;
	    sep=MEANLEGSEP*2;
	}
	if (sep<MEANLEGSEP/2) {
	    dbg("LegStats",2) << "Leg separation too low at " << sep << "; increasing to " << MEANLEGSEP/2 << std::endl;
	    sep=MEANLEGSEP/2;
	}
    }
}

void LegStats::updateDiameter(float newDiam, float newDiamSEM) {
    // TODO: track diamSigma
    diam = diam*(1-1/LEGSTATSTC) + newDiam/LEGSTATSTC;
    dbg("LegStats.updateDiameter",3) << "newDiam=" << newDiam << ", updated diam=" << diam << ", sigma=" << diamSigma << std::endl;
}

