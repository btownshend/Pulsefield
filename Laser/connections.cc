#include <string>
#include "person.h"
#include "connections.h"
#include "dbg.h"
#include "conductor.h"

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

    if (TouchOSC::instance()->isFrozen()) {
	dbg("Connections.handlOSCMessage",2) << "Frozen" << std::endl;
	return 0;
    }

    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"conductor")==0) {
	Conductor::instance()->setHostname(lo_address_get_hostname(lo_message_get_source(msg)));
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
	} else if (attributes.isSet("fusion")) {
	    drawFusion(d,p1,p2,attributes.get("fusion").getValue());
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

// Draw fusion at give stage (0=unfused,1.0=fully fused)
void Connection::drawFusion(Drawing &d, const Person *p1, const Person *p2, float stage) const {
    Point c1=p1->get();  float r1=p1->getBodyDiam()/2;
    Point c2=p2->get();  float r2=p2->getBodyDiam()/2;
    if (c1==c2) {
	dbg("Connection.drawFusion",1) << "Points are identical -- skipping draw" << std::endl;
	return;
    }
    Point center=(c1+c2)/2;
    float rpinch=stage*(r1+r2)/2;
    // Build the beziers clockwise starting/ending in center (so the 2 halves connect)
    drawFusionHalf(d,c1,r1,center,rpinch);
    drawFusionHalf(d,c2,r2,center,rpinch);
}

void Connection::drawFusionHalf(Drawing &d, Point c, float r, Point ctr, float rpinch) const {
    //            9       A      B
    //          8                      C         D
    //          7                 
    //          6                     2         1
    //           5        4       3
    //   Beziers:   1234, 4556, 6778, 89AB
    float k=4*(sqrt(2)-1)/3;    // Optimal offset to make bezier approximate a circle
    Point right=ctr-c; right=right/right.norm();
    Point up=right.rot90();
    Point mid=(ctr*2.0 + (c+right*r))/3.0f;
    Point p1(ctr-up*rpinch);
    Point p2(mid-up*rpinch);
    Point p3(c-up*r+right*r);
    Point p4(c-up*r);
    Point p5(c-up*r-right*r*k);
    Point p6(c-up*r*k-right*r);
    Point p7(c-right*r);
    Point p8(c-right*r+up*r*k);
    Point p9(c-right*r*k+up*r);
    Point pA(c+up*r);
    Point pB(c+up*r+right*r);
    Point pC(mid+up*rpinch);
    Point pD(ctr+up*rpinch);
    Color col(0.0,1.0,0.0);
    dbg("Connection.drawFusionHalf",3) << "c=" << c << ", r=" << r << ", ctr=" << ctr << ", rpinch=" << rpinch << std::endl;
    dbg("Connection.drawFusionHalf",3) << "right=" << right << ", up=" << up << ", mid=" << mid << std::endl;
    dbg("Connection.drawFusionHalf",3) << p1 << p2 << p3 << p4 << p5 << p6 << p7 << p8 << p9 << pA << pB << pC << pD << std::endl;
    d.drawCubic(p1,p2,p3,p4,col);
    d.drawCubic(p4,p5,p6,p7,col);
    d.drawCubic(p7,p8,p9,pA,col);
    d.drawCubic(pA,pB,pC,pD,col);
}

