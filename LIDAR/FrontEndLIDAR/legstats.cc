#include <iomanip>
#include <math.h>
#include "person.h"
#include "legstats.h"
#include "parameters.h"

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
      << ",sep: " << ls.sep;
    return s;
}

void LegStats::update(const Person &p) {
    Point legdiff=p.getLeg(1).getPosition()-p.getLeg(0).getPosition();
    leftness=leftness*(1-1/LEFTNESSTC)+legdiff.dot(Point(-p.getVelocity().Y(),p.getVelocity().X()))/LEFTNESSTC;
    // TODO: update other stats
}


