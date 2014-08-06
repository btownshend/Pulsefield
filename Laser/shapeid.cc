#include "shapeid.h"

std::map< std::string, std::shared_ptr<ShapeID> > ShapeIDs::ids;


void ShapeIDs::frameTick(int frame) {
    static const int MAXAGE=10;   // Number of frames a shape ID is unseen before erasing
    for (std::map<std::string, std::shared_ptr<ShapeID> >::iterator s=ids.begin();s!=ids.end();) {
	s->second->incrementAge();
	if (s->second->getAge() > MAXAGE) {
	    std::map<std::string, std::shared_ptr<ShapeID> >::iterator toerase=s;
	    dbg("ShapeIDs.frameTick",1) << "Erasing stale shapeID " << s->second->getID() << std::endl;
	    s++;
	    ids.erase(toerase);
	} else {
	    s++;
	}
    }
}

