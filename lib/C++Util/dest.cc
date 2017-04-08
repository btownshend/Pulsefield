#include <iostream>
#include <string.h>
#include "dest.h"
#include "dbg.h"

std::ostream& operator<<(std::ostream &s, const Destination &d) {
    s << d.host << ":" << d.port;
    return s;
}

Destinations::Destinations() {
}

Destinations::~Destinations() {
    removeAll();
}

void Destinations::add(const char *host, int port) {
    for (unsigned int i=0;i<dests.size();i++)
	if (strcmp(dests[i].host.c_str(),host)==0 && dests[i].port==port)   {
	    dbg("Destinations.add",1) << "Already have " << host << ":" << port << " as a destination" << std::endl;
	    return;
	}
    Destination d(host,port);
    dests.push_back(d);
    dbg("Destinations.add",1) << "Added destination " << d << std::endl;
}

void Destinations::remove(const char *host, int port) {
    for (unsigned int i=0;i<dests.size();i++)
	if (strcmp(dests[i].host.c_str(),host)==0 && dests[i].port==port)  {
	    remove(i);
	    i--;
	}
}
	
void Destinations::remove(int i) {
    dbg("Destinations.remove",1) << "Removed destination " << dests[i] << std::endl;
    std::cout << "Removed destination " << dests[i] << std::endl;
    dests.erase(dests.begin()+i);
}
	
void Destinations::removeAll() {
    dests.clear();
    dbg("Destinations.removeAll",1) << "Removed all destinations." << std::endl;
}
