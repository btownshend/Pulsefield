#include <string>
#include "connections.h"
#include "dbg.h"

Connections *Connections::theInstance=NULL;

std::ostream &operator<<(std::ostream &s, const ConnectionAttribute &c) {
    s << c.getValue() << "@" << c.getTime();
    return s;
}

std::ostream &operator<<(std::ostream &s, const Connection &c) {
    s << c.cid << " [" << c.uid[0] << "," << c.uid[1] << "] ";
    for (std::map<std::string,ConnectionAttribute>::const_iterator a=c.attributes.begin(); a!=c.attributes.end();a++) {
	s << a->first << ": " << a->second << " ";
    }
    return s;
}

std::ostream &operator<<(std::ostream &s, const Connections &c) {
    for (std::map<CIDType,Connection>::const_iterator a=c.conns.begin(); a!=c.conns.end();a++) {
	s << a->first << ": " << a->second << std::endl;
    }
    return s;
}

int Connections::handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Connections.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << lo_address_get_url(lo_message_get_source(msg)) << std::endl;

    const char *tok=strtok((char *)path,"/");
    bool handled=false;
    if (strcmp(tok,"conductor")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"conx")==0) {
	    if (strcmp(types,"sssiiff")!=0) {
		dbg("Connections.handleOSCMessage",1) << "/conductor/conx has unexpected types: " << types << std::endl;
		return 0;
	    }
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
    
    return handled?0:1;
}

void Connections::incrementAge() {
    for (std::map<CIDType,Connection>::iterator a=conns.begin(); a!=conns.end();a++) {
	a->second.incrementAge();
	if (a->second.getAge() > MAXAGE) {
	    dbg("Connections.incrementAge",1) << "Connection " << a->first << " has age " << a->second.getAge() << "; deleting." << std::endl;
	    conns.erase(a);
	}
    }
}
