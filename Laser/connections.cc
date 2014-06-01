#include <string>
#include "connections.h"
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
		dbg("Connections.handleOSCMessage",1) << "/conductor/conx has unexpected types: " << types << std::endl;
	    } else {
		std::string type=&argv[0]->s;
		std::string subtype=&argv[1]->s;
		CIDType cid=&argv[2]->s;
		int uid1=argv[3]->i;
		int uid2=argv[4]->i;
		float value=argv[5]->f;
		float time=argv[6]->f;
		dbg("Connections.handleOSCMessage",1) << "type=" << type << ", subtype=" << subtype << ", cid=" << cid << ",uids=" << uid1 << "," << uid2 << ", value=" << value << ", time=" << time << std::endl;
		if (conns.count(cid)==0)
		    conns[cid]=Connection(cid,uid1,uid2);
		conns[cid].set(type,subtype,value,time);
		handled=true;
	    }
	}
	else if (strcmp(tok,"rollcall")==0) {
	    int uid=argv[0]->i;
	    std::string action=&argv[1]->s;
	    int numconx=argv[2]->i;
	    dbg("Connections.handleOSCMessage",1) << "rollcall:  uid=" << uid << ", action=" << action << ", numconx=" << numconx << std::endl << *this;
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
