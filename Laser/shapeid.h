#include <string>
#include <map>

#include "dbg.h"

// ShapeID's are unique ID's assigned to particular pieces of geometry, which are
// persistent over multiple frames.   They are primarily used to aid in laser allocation to
// usually use the same laser for the same shape over multiple frames to avoid jumping
// around.
class ShapeID {
    std::string id;
    int laser;
    int age;
 public:
    ShapeID(const std::string &id) { this->id=id; laser=-1; age=0; }
    const std::string &getID() const { return id; }
    void setLaser(int lnum)  { age=0; laser=lnum; }
    void incrementAge() { age++; }
    int getAge() const { return age; }
    int getLaser() const {return laser; }
};

class ShapeIDs {
    static std::map< std::string, std::shared_ptr<ShapeID> > ids;
 public:
    static std::shared_ptr<ShapeID> get(const std::string &id) {
	if (ids.count(id)==0) {
	    dbg("ShapeIDs.get",1) << "Creating new ShapeID: " << id << std::endl;
	    ids[id]=std::shared_ptr<ShapeID>(new ShapeID(id));
	}
	return ids[id];
    }
    static void frameTick(int frame);
};


	
