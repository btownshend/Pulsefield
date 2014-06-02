#include <string>
#include "groups.h"
#include "dbg.h"
#include "person.h"

Groups *Groups::theInstance=NULL;

Group *Groups::getGroup(int id) {
    if (p.count(id)==0)
	return NULL;
    return &p.at(id);
}

Group *Groups::getOrCreateGroup(int id) {
    if (p.count(id)==0)
	p.insert(std::pair<int,Group>(id,Group(id)));
    return &p.at(id);
}

int Groups::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Groups.handleOSCMessage",3)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");
    bool handled=false;
    if (strcmp(tok,"pf")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"group")==0) {
	    if (strcmp(types,"iiiffff")!=0) {
		dbg("Groups.handleOSCMessage",1) << path << " has unexpected types: " << types << std::endl;
	    } else {
		int id=argv[1]->i;
		Point position(argv[4]->f,argv[5]->f);
		float diameter=argv[6]->f;
		dbg("Groups.group",1) << "id=" << id << ",pos=" << position << std::endl;
		Group *group=getOrCreateGroup(id);
		group->set(position);
		group->setDiameter(diameter);
		handled=true;
	    }
	}
    } else if (strcmp(tok,"/conductor")) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"gattr")==0) {
	    std::string type=&argv[0]->s;
	    int gid=argv[1]->i;
	    float value=argv[2]->f;
	    float time=argv[3]->f;
	    dbg("Groups.gattr",1) << "Set attribute " << type << " for gid " << gid << ": value=" << value << ", time=" << time << std::endl;
	    Group *group = getGroup(gid);
	    if (group==NULL) {
		dbg("Groups.gattr",1) << "Group " << gid << " not found -- ignoring conductor message" << std::endl;
	    } else
		group->set(type,value,time);
	    handled=true;
	}
    }

    if (!handled) {
	dbg("Groups.handleOSCMessage",1) << "Unhanded message: " << path << "(" << types << "): parse failed at token: " << tok << std::endl;
    }
    delete [] pathCopy;
    return handled?0:1;
}

void Groups::incrementAge_impl() {
    int initialCount=p.size();
    for (std::map<int,Group>::iterator a=p.begin(); a!=p.end();) {
	a->second.incrementAge();
	if (a->second.getGroupSize()==0 || a->second.getAge() > MAXAGE) {
	    dbg("Groups.incrementAge",1) << "Group  " << a->first << " has age " << a->second.getAge() << " with " << a->second.getGroupSize() << " people; deleting." << std::endl;
	    std::map<int,Group>::iterator toerase=a;
	    a++;
	    p.erase(toerase);
	} else
	    a++;
    }
    if (p.size() != initialCount) {
	dbg("Groups.incrementAge",1) << "Reduced number of groups from " << initialCount << " to " << p.size() << std::endl;
    }
}

void Groups::draw_impl(Drawing &d)  const {
    dbg("Groups.draw",3) << "Draw for " << p.size() << " groups." << std::endl;
    if (TouchOSC::instance()->isBodyEnabled() || TouchOSC::instance()->isLegsEnabled()) {
	for (std::map<int,Group>::const_iterator a=p.begin(); a!=p.end();a++)
	    a->second.draw(d);
    }
}

void Group::draw(Drawing &d) const  {
    dbg("Group.draw",3) << "Draw group " << id << " with " << getGroupSize() << " people." << std::endl;
    if (ids.size() > 1) {
	d.shapeBegin(attributes,true); // Create  a composite drawing that we'll later image as a convex hull
	// Draw all the person circles (will later be converted into a hull)
	for (std::set<int>::const_iterator a=ids.begin();a!=ids.end();a++) {
	    const Person *p=People::instance()->getPerson(*a);
	    if (p!=NULL)
		p->drawBody(d);
	}
	// d.drawCircle(position,diameter/2,Color(0.0,1.0,0.0));
	d.shapeEnd();
    }
}
