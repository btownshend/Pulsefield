#include "groups.h"
#include "person.h"
#include "dbg.h"
#include "parameters.h"

std::ostream &operator<<(std::ostream &s, const Group &g) {
    s << "GID: " << g.gid;
    return s;
}

Group::~Group() {
    assert(members.size()==0);
}

// Get all other people that are connected by <=maxdist to person i
std::set<int> Groups::getConnected(int i, std::set<int> current,const std::vector<Person> &people) {
    current.insert(i);
    for (unsigned int j=0;j<people.size();j++)  {
	float d=(people[i].getPosition() - people[j].getPosition()).norm();
	if (current.count(j)==0 && (d <= groupDist || (d<=unGroupDist && people[i].isGrouped() && people[i].getGroup()==people[j].getGroup()))) {
	    current=getConnected(j,current,people);
	}
    }
    dbg("Groups.getConnected",10) << "getConnected(" << i << ") -> " << current.size() << std::endl;
    return current;
}

int Groups::nextID() const {
    int id=1;
    for (std::set<Group*>::iterator g=groups.begin();g!=groups.end(); g++)
	id=std::max((*g)->getID()+1,id);
    return id;
}

Group *Groups::newGroup(double elapsed) {
    Group *grp=new Group(nextID(),elapsed);
    dbg("Groups",2) << "Create new group " << grp->getID() << std::endl;
    groups.insert(grp);
    return grp;
}

// Update groups
void Groups::update(std::vector<Person> &people, double elapsed) {
    std::vector<bool> scanned(people.size(),0);
    std::set<Group *> usedGroups;

    for (unsigned int i=0;i<people.size();i++) {
	if (!scanned[i]) {
	    // Person is in a group, but not yet accounted for
	    // Find their connected set (none of these will be accounted for yet)
	    std::set<int> connected = getConnected(i,std::set<int>(),people);
	    if (connected.size() > 1) {
		Group *grp=NULL;
		for (std::set<int>::iterator c=connected.begin();c!=connected.end();c++) 
		    if (people[*c].isGrouped() && usedGroups.count(people[*c].getGroup())==0) {
			grp=people[*c].getGroup();
			break;
		    }

		if (grp==NULL)
		    // Need to allocate a new group
		    grp = newGroup(elapsed);

		// Assign it
		Point centroid;
		for (std::set<int>::iterator c=connected.begin();c!=connected.end();c++) {
		    assert(!scanned[*c]);
		    scanned[*c]=true;
		    if (!people[*c].isGrouped()) {
			dbg("Groups.update",2) << "Assigning  person " << people[*c].getID()  << " to " << *grp << std::endl;
		    } else if (people[*c].getGroup() != grp) {
			dbg("Groups.update",2) << "Moving  person " << people[*c].getID()  << " from  " << *people[*c].getGroup() << " to " << *grp << std::endl;
		    }
		    people[*c].addToGroup(grp);
		    centroid=centroid+people[*c].getPosition();
		}
		centroid = centroid/connected.size();
		grp->setCentroid(centroid);
		// Compute diameter of group 
		float diameter=0;
		for (std::set<int>::iterator c=connected.begin();c!=connected.end();c++) {
		    diameter+=(people[*c].getPosition()-centroid).norm();
		}
		diameter = diameter/connected.size();
		grp->setDiameter(diameter);

		usedGroups.insert(grp);  // Make this group as used so we don't use it for any other people that might have been in this group
	    } else if (people[i].isGrouped()) {
		// Unconnected person
		dbg("Groups.update",2) << "Ungrouping person " << people[i].getID() << " from  " << *people[i].getGroup() << std::endl;
		people[i].unGroup();
	    }
	}
    }

    // Remove any old groups with nobody in them
    for (std::set<Group*>::iterator g=groups.begin();g!=groups.end();) {
	// Copy next position so we can safely delete g
	std::set<Group *>::iterator nextIt = g; nextIt++;
	if ((*g)->size() == 0) {
	    delete (*g);
	    groups.erase(g);
	}
	g=nextIt;
    }
}


void Groups::sendMessages(lo_address &addr, int frame, double elapsed) const {
    if (groups.size()==0)
	return;
    dbg("Groups.sendMessages",2) << "Frame " << frame << ": sending " << groups.size() << " group messages" << std::endl;
    for (std::set<Group*>::iterator g=groups.begin();g!=groups.end(); g++)
	(*g)->sendMessages(addr,frame,elapsed);
}

void Group::sendMessages(lo_address &addr, int frame, double elapsed) const {
    lo_send(addr,"/pf/group","iiiffff",frame,gid,size(),elapsed-createTime,centroid.X()/UNITSPERM, centroid.Y()/UNITSPERM, diameter/UNITSPERM);
}
