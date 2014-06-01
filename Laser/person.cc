
#include <string>
#include "person.h"
#include "dbg.h"

People *People::theInstance=NULL;

Person *People::getPerson(int id) {
    if (p.count(id)==0)
	p.insert(std::pair<int,Person>(id,Person(id)));
    return &p.at(id);
}

int People::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("People.handleOSCMessage",3)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");
    bool handled=false;
    if (strcmp(tok,"pf")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"body")==0) {
	    if (strcmp(types,"iifffffffffffffffi")!=0) {
		dbg("People.handleOSCMessage",1) << path << " has unexpected types: " << types << std::endl;
	    } else {
		int id=argv[1]->i;
		Point position(argv[2]->f,argv[3]->f);
		dbg("People.handleOSCMessage",1) << "id=" << id << ",pos=" << position << std::endl;
		Person *person=getPerson(id);
		person->set(position);
		person->setStats(argv[12]->f,argv[14]->f);
		handled=true;
	    }
	}
	else if (strcmp(tok,"leg")==0) {
	    int id=argv[1]->i;
	    int leg=argv[2]->i;
	    Point position(argv[4]->f,argv[5]->f);
	    Person *person=getPerson(id);
	    person->setLeg(leg,position);
	    dbg("People.handleOSCMessage",1) << "id=" << id << ", leg=" << leg << ", pos=" << position << std::endl;
	    handled=true;
	}
	else if (strcmp(tok,"body")==0) {
	    int id=argv[2]->i;
	    int gid=argv[9]->i;
	    int gsize=argv[10]->i;
	    Person *person=getPerson(id);
	    person->setGrouping(gid,gsize);
	    dbg("People.handleOSCMessage",1) << "id=" << id << ", group=" << gid << " with " << gsize << " people" << std::endl;
	    handled=true;
	}
	else if (strcmp(tok,"update")==0) {
	    // Not needed
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
    for (std::map<int,Person>::iterator a=p.begin(); a!=p.end();) {
	a->second.incrementAge();
	if (a->second.getAge() > MAXAGE) {
	    dbg("People.incrementAge",1) << "Connection " << a->first << " has age " << a->second.getAge() << "; deleting." << std::endl;
	    std::map<int,Person>::iterator toerase=a;
	    a++;
	    p.erase(toerase);
	} else
	    a++;
    }
}

void People::draw_impl(Drawing &d, bool drawBody, bool drawLegs)  const {
    dbg("People.draw",3) << "Draw(" << drawBody << "," << drawLegs << ") for " << p.size() << " people." << std::endl;
    if (drawBody || drawLegs) {
	for (std::map<int,Person>::const_iterator a=p.begin(); a!=p.end();a++) {
	    d.shapeBegin();
	    a->second.draw(d,drawBody,drawLegs);
	    d.shapeEnd();
	}
    }
}

void Person::draw(Drawing &d, bool drawBody, bool drawLegs) const  {
    if (drawBody) {
	d.drawCircle(position,legDiam+legSep,Color(0.0,1.0,0.0));
    }
    if (drawLegs) {
	for (int i=0;i<2;i++)
	    d.drawCircle(legs[i].get(),legDiam,Color(0.0,1.0,0.0));
    }
}
