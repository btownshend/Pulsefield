#include <string>
#include "connections.h"
#include "person.h"
#include "dbg.h"

Connections *Connections::theInstance=NULL;

std::ostream &operator<<(std::ostream &s, const Connection &c) {
    s << c.cid << " [" << c.uid[0] << "," << c.uid[1] << "] " << c.attributes;
    return s;
}

std::ostream &operator<<(std::ostream &s, const Connections &c) {
    for (std::map<CIDType,Connection>::const_iterator a=c.conns.begin(); a!=c.conns.end();a++) {
	s << a->first << ": " << a->second << std::endl;
    }
    return s;
}

int Connections::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Connections.handleOSCMessage",3)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;

    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"conductor")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"conx")==0) {
	    if (strcmp(types,"sssiiff")!=0) {
		dbg("Connections.conx",1) << "/conductor/conx has unexpected types: " << types << std::endl;
	    } else {
		std::string type=&argv[0]->s;
		std::string subtype=&argv[1]->s;
		CIDType cid=&argv[2]->s;
		int uid1=argv[3]->i;
		int uid2=argv[4]->i;
		float value=argv[5]->f;
		float time=argv[6]->f;
		dbg("Connections.conx",1) << "type=" << type << ", subtype=" << subtype << ", cid=" << cid << ",uids=" << uid1 << "," << uid2 << ", value=" << value << ", time=" << time << std::endl;
		if (People::personExists(uid1) && People::personExists(uid2)) {
		    if (conns.count(cid)==0)
			conns[cid]=Connection(cid,uid1,uid2);
		    conns[cid].set(type,subtype,value,time);
		} else {
		    dbg("Connections.conx",1) << "Connection " << cid << " " << type << "." << subtype << " between missing people: " << uid1 << "," << uid2 <<  " ignored" << std::endl;
		}
		handled=true;
	    }
	}
	else if (strcmp(tok,"rollcall")==0) {
	    int uid=argv[0]->i;
	    std::string action=&argv[1]->s;
	    int numconx=argv[2]->i;
	    dbg("Connections.rollcall",2) << "rollcall:  uid=" << uid << ", action=" << action << ", numconx=" << numconx << std::endl;
	    if (!People::personExists(uid)) {
		dbg("Connections.rollcall",1) << "rollcall has uid for which there is no record: " << uid << std::endl << *this;
	    }
	    handled=true;
	}
    }
    if (!handled) {
	dbg("Connections.handleOSCMessage",1) << "Unhanded message: " << path << ": parse failed at token: " << tok << std::endl;
    }
    
    delete [] pathCopy;
    return handled?0:1;
}

void Connections::incrementAge_impl() {
    for (std::map<CIDType,Connection>::iterator a=conns.begin(); a!=conns.end();) {
	a->second.incrementAge();
	if (a->second.getAge() > MAXAGE) {
	    dbg("Connections.incrementAge",1) << "Connection " << a->first << " has age " << a->second.getAge() << "; deleting. (had " << conns.size() << " entries)" << std::endl;
	    std::map<CIDType,Connection>::iterator toerase=a;
	    a++;
	    conns.erase(toerase);
	} else if (!People::personExists(a->second.getUID(0)) || !People::personExists(a->second.getUID(1))) {
	    dbg("Connections.incrementAge",1) << "Connection " << a->first << " is between non-existent people  " << a->second.getUID(0) << " and " << a->second.getUID(1) << "; deleting. (had " << conns.size() << " entries)" << std::endl;
	    std::map<CIDType,Connection>::iterator toerase=a;
	    a++;
	    conns.erase(toerase);
	} else
	    a++;
    }
    dbg("Connections.incrementAge",6) << "Connections now have " << conns.size() << " entries" << std::endl;
}

void Connections::draw_impl(Drawing &d) const {
    dbg("Connections.draw",3) << "Drawing " << conns.size() << " connections." << std::endl;
    for (std::map<CIDType,Connection>::const_iterator a=conns.begin(); a!=conns.end();a++)
	a->second.draw(d);
}


void Connection::draw(Drawing &d) const {
    d.shapeBegin(attributes);
    if (visual.getNumElements()==0) {
	dbg("Connection.draw",3) << "Using internal draw" << std::endl;
	Person *p1= People::instance()->getPerson(uid[0]);
	Person *p2= People::instance()->getPerson(uid[1]);
	if (p1==NULL || p2==NULL) {
	    dbg("Connection.draw",1) << "Connection " << cid << " is invalid -- at least one UID is missing" << std::endl;
	} else {
	    Point pt1=p1->get();
	    Point pt2=p2->get();
	    Point delta=pt2-pt1;
	    delta=delta/delta.norm();
	    // Move onto radius of person
	    pt1=pt1+delta*p1->getBodyDiam()/2;
	    pt2=pt2-delta*p1->getBodyDiam()/2;
	    Point pt3,pt4;
	    Point mid=(pt1+pt2)/2;
	    if (fabs(delta.X()) > fabs(delta.Y())) {
		pt3=Point(mid.X(),pt1.Y());
		pt4=Point(mid.X(),pt2.Y());
	    } else {
		pt3=Point(pt1.X(),mid.Y());
		pt4=Point(pt2.X(),mid.Y());
	    }
	    d.drawCubic(pt1,pt3,pt4,pt2,Color(0.0,1.0,0.0));
	}
    } else {
	dbg("Connection.draw",3) << "Using received visual with " << visual.getNumElements() << " elements." << std::endl;
	d.append(visual);
    }
    d.shapeEnd();
}
