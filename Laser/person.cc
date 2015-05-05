
#include <string>
#include "person.h"
#include "groups.h"
#include "dbg.h"
#include "lasers.h"

People *People::theInstance=NULL;

Person *People::getPerson(int id) {
    if (p.count(id)==0)
	return NULL;
    return &p.at(id);
}

Person *People::getOrCreatePerson(int id) {
    if (p.count(id)==0)
	p.insert(std::pair<int,Person>(id,Person(id)));
    return &p.at(id);
}

int People::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("People.handleOSCMessage",3)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;
    if (TouchOSC::instance()->isFrozen()) {
	dbg("People.handlOSCMessage",2) << "Frozen" << std::endl;
	return 0;
    }
	
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");
    bool handled=false;
    if (strcmp(tok,"pf")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"body")==0) {
	    if (strcmp(types,"iifffffffffffffffi")!=0) {
		dbg("People.body",1) << path << " has unexpected types: " << types << std::endl;
	    } else {
		int id=argv[1]->i;
		Point position(argv[2]->f,argv[3]->f);
		dbg("People.body",1) << "id=" << id << ",pos=" << position << std::endl;
		Person *person=getOrCreatePerson(id);
		person->set(position);
		person->setStats(argv[12]->f,argv[14]->f);
		person->setFacing(argv[10]->f);
		handled=true;
	    }
	}
	else if (strcmp(tok,"leg")==0) {
	    int id=argv[1]->i;
	    int leg=argv[2]->i;
	    Point position(argv[4]->f,argv[5]->f);
	    Person *person=getOrCreatePerson(id);
	    person->setLeg(leg,position);
	    dbg("People.leg",1) << "id=" << id << ", leg=" << leg << ", pos=" << position << std::endl;
	    handled=true;
	}
	else if (strcmp(tok,"update")==0) {
	    if (strcmp(types,"ififfffffiii")!=0) {
		dbg("People.handleOSCMessage",1) << path << " has unexpected types: " << types << std::endl;
	    } else {
		int id=argv[2]->i;
		int gid=argv[9]->i;
		int gsize=argv[10]->i;
		Person *person=getOrCreatePerson(id);
		if (person->getGroupID() != gid) {
		    Group *oldGroup = Groups::instance()->getGroup(person->getGroupID());
		    if (oldGroup!=NULL) {
			oldGroup->removePerson(id);
			dbg("People.update",1) << "Removing id " << id << " from group=" << person->getGroupID() << std::endl;
		    }
		    if (gid!=0) {
			dbg("People.update",1) << "Adding id " << id << " to group " << gid << std::endl;
			Group *group=Groups::instance()->getOrCreateGroup(gid);
			group->addPerson(id);
			if (group->getGroupSize() != gsize) {
			    dbg("People.update",1) << "Group " << gid << " has " << group->getGroupSize() << " people, but update message indicates there should be " << gsize << std::endl;
			}
			person->setGrouping(gid,gsize);
		    } else 
			person->setGrouping(0,1);
		}
		dbg("People.update",1) << "id=" << id << ", group=" << gid << " with " << gsize << " people" << std::endl;
		handled=true;
	    }
	}
	else if (strcmp(tok,"update")==0) {
	    // Not needed
	    handled=true;
	}
    } else if (strcmp(tok,"/conductor")) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"attr")==0) {
	    std::string type=&argv[0]->s;
	    int uid=argv[1]->i;
	    float value=argv[2]->f;
	    float time=argv[3]->f;
	    float freshness=argv[4]->f;
	    dbg("People.handleOSCMessage",1) << "Set attribute " << type << " for id " << uid << ": value=" << value << ", time=" << time << ", freshness=" << freshness << std::endl;
	    Person *person = getPerson(uid);
	    if (person==NULL) {
		dbg("People.handleOSCMessage",1) << "Person " << uid << " not found -- ignoring conductor message" << std::endl;
	    } else
		person->set(type,value,time);
	    handled=true;
	}
    }

    if (!handled) {
	dbg("People.handleOSCMessage",1) << "Unhanded message: " << path << "(" << types << "): parse failed at token: " << tok << std::endl;
    }
    delete [] pathCopy;
    return handled?0:1;
}

void People::incrementAge_impl() {
    int initialCount=p.size();
    for (std::map<int,Person>::iterator a=p.begin(); a!=p.end();) {
	a->second.incrementAge();
	if (a->second.getAge() > MAXAGE) {
	    dbg("People.incrementAge",1) << "Person  " << a->first << " has age " << a->second.getAge() << "; deleting." << std::endl;
	    std::map<int,Person>::iterator toerase=a;
	    a++;
	    p.erase(toerase);
	} else
	    a++;
    }
    if (p.size() != initialCount) {
	dbg("People.incrementAge",1) << "Reduced number of people from " << initialCount << " to " << p.size() << std::endl;
    }
}

void People::draw_impl(Drawing &d)  const {
    dbg("People.draw",3) << "Draw for " << p.size() << " people." << std::endl;
    for (std::map<int,Person>::const_iterator a=p.begin(); a!=p.end();a++)
	a->second.draw(d);
}

void Person::draw(Drawing &d) const  {
    d.shapeBegin("cell:"+std::to_string(id),attributes);
    if (Lasers::instance()->getFlag("body")) {
	drawBody(d);
    }
    if (Lasers::instance()->getFlag("legs")) {
	drawLegs(d);
    }
    if (visual!=nullptr) {
	dbg("Person.draw",1) << "Drawing visual with " << visual->getNumElements() << " elements on drawing with " << d.getNumElements() << " elements at " << position << " after translating it by " << position-visPosition << std::endl;
	visual->translate(position-visPosition);
	((Person *)this)->visPosition=position;  // Need to override CONST
	for (int i=0;i<visual->getNumElements();i++)
	    d.append(visual->getElement(i));  // Appends to current composite
	dbg("Person.draw",1) << "Drawing now has " << d.getNumElements() << " elements" << std::endl;
    }
    d.shapeEnd("cell:"+std::to_string(id));
    if (TouchOSC::instance()->isLabelsEnabled()) {
	d.shapeBegin("cell:"+std::to_string(id)+"-attr",Attributes());
	Point facingVector;
	facingVector.setThetaRange(facing*M_PI/180,1.0);
	Point orthVector=facingVector.rotate(M_PI/2);
	Point p1=position+(facingVector*0.35)-orthVector*0.25;
	Point p2=position+(facingVector*0.35)+orthVector*0.25;
	attributes.drawLabels(d,p1,p2);
	d.shapeEnd("cell:"+std::to_string(id)+"-attr");
    }
}

void Person::drawBody(Drawing &d) const {
    d.drawCircle(position,getBodyDiam()/2,Color(0.0,1.0,0.0));
}

void Person::drawLegs(Drawing &d) const  {
    for (int i=0;i<2;i++)
	d.drawCircle(legs[i].get(),legDiam/2,Color(0.0,1.0,0.0));
}
