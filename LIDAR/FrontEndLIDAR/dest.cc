#include <stdio.h>
#include <string.h>
#include "dest.h"
#include "dbg.h"

Destinations::Destinations() {
    ndest=0; maxdest=0;
    hosts = (char **)0;
    ports = (int *)0;
}

Destinations::~Destinations() {
    removeAll();
    if (maxdest>0) {
	delete [] hosts;
	delete [] ports;
    }
}

void Destinations::add(const char *host, int port) {
    for (int i=0;i<ndest;i++)
	if (strcmp(hosts[i],host)==0 && ports[i]==port)   {
	    dbg("Destinations.add",1) << "Already have " << host << ":" << port << " as a destination" << std::endl;
	    return;
	}
	    
    if (ndest==maxdest) {
	// Reallocate space
	maxdest=maxdest+10;
	char **newhosts=new char *[maxdest];
	memcpy(newhosts,hosts,sizeof(*hosts)*ndest);
	if (hosts)
	    delete [] hosts;
	hosts=newhosts;
	int *newports = new int[maxdest];
	memcpy(newports, ports, sizeof(*ports)*ndest);
	if (ports)
	    delete [] ports;
	ports=newports;
    }
    hosts[ndest]=new char[strlen(host)+1];
    strcpy(hosts[ndest],host);
    ports[ndest]=port;
    dbg("Destinations.add",1) << "Added destination " << host << ":" << port << std::endl;
    ndest++;
}

void Destinations::remove(const char *host, int port) {
    for (int i=0;i<ndest;i++)
	if (strcmp(hosts[i],host)==0 && ports[i]==port)  {
	    for (int j=i;j<ndest+1;j++) {
		hosts[j]=hosts[j+1];
		ports[j]=ports[j+1];
	    }
	    ndest--;
	    delete [] hosts[ndest];
	    i--;
	    dbg("Destinations.remove",1) << "Removed destination " << host << ":" << port << std::endl;
	    printf("Removed destination %s:%d\n", host,port);
	}
}
	
void Destinations::removeAll() {
    for (int i=0;i<ndest;i++) 
	delete [] hosts[i];
    ndest=0;
    dbg("Destinations.removeAll",1) << "Removed all destinations." << std::endl;
}
