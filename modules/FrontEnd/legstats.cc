#include <iomanip>
#include <math.h>
#include "person.h"
#include "legstats.h"
#include "parameters.h"
#include "dbg.h"

LegStats::LegStats() {
    sep=MEANLEGSEP;
    sepSigma=LEGSEPSIGMA;
    leftness=0.0;
    facing=0.0;
    facingSEM=FACINGSEM;
    updateSep=false;
    if (!updateSep) {
	dbg("LegStats",1) << "Not updating leg seps" << std::endl;
    }
}

std::ostream &operator<<(std::ostream &s, const LegStats &ls) {
    s << std::fixed << std::setprecision(0)
      << ",sep: " << ls.sep
      << std::setprecision(3);
    return s;
}

void LegStats::update(const Person &p) {
    // Update leftness
    Point legdiff=p.getLeg(1).getPosition()-p.getLeg(0).getPosition();
    leftness=leftness*(1-1.0f/LEFTNESSTC)+legdiff.dot(Point(-p.getVelocity().Y(),p.getVelocity().X()))/LEFTNESSTC;

    // Update facing
    float curfacing=legdiff.getTheta()+M_PI/2;
    if (leftness>0)
	curfacing-=M_PI;
    float diff=curfacing-facing;
    while (diff>M_PI)
	diff-=2*M_PI;
    while (diff<-M_PI)
	diff+=2*M_PI;
    dbg("LegStats",4) << "Update leftness of " << facing*180/M_PI << " towards " << curfacing*180/M_PI << " to " << (facing+diff/FACINGTC)*180/M_PI << std::endl;
    facing += diff/FACINGTC;
    while (facing>M_PI)
	facing-=2*M_PI;
    while (facing<-M_PI)
	facing+=2*M_PI;
    
    // Update separation
    if (updateSep && p.getLeg(0).isVisible() && p.getLeg(1).isVisible()) {
	// Both legs visible, update separation estimate
	float cursep=(p.getLeg(0).getPosition()-p.getLeg(1).getPosition()).norm();
	float alphaSep=1.0f/LEGSEPTC;
	float oldsep=sep;
	sep = sep*(1-alphaSep) + alphaSep*cursep;
	// TODO: track sepSigma
	if (sep>MAXLEGSEP) {
	    dbg("LegStats",2) << "Leg separation too high at " << sep << "; reducing to " << MAXLEGSEP << std::endl;
	    sep=MAXLEGSEP;
	}
	if (sep<MINLEGSEP) {
	    dbg("LegStats",2) << "Leg separation too low at " << sep << "; increasing to " << MINLEGSEP << std::endl;
	    sep=MINLEGSEP;
	}
	float alphaSepSigma=1.0f/LEGSEPSIGMATC;
	sepSigma=sqrt(sepSigma*sepSigma*(1-alphaSepSigma)+alphaSepSigma*(cursep-oldsep)*(cursep-oldsep));
	dbg("LegStats.update",2) << "Current sep=" << cursep << ", mean sep= " << sep << " +/- " << sepSigma << std::endl;
    }
}


