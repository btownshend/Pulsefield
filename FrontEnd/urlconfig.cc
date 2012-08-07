#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include "urlconfig.h"

URLConfig::URLConfig(const char *configFile) {
    filename=new char[strlen(configFile)+1];
    strcpy(filename,configFile);
    FILE *fd=fopen(configFile,"r");
    if (fd==NULL) {
	fprintf(stderr,"Unable to open url config file: %s\n", configFile);
	exit(1);
    }
    for (int i=0;i<MAXURLS;i++) {
	int nread=fscanf(fd,"%100[^,],%100[^,],%d\n",idents[i],hosts[i],&ports[i]);
	if  (nread<0) {
	    nurl=i;
	    return;
	}
	if (nread!=3) {
	    fprintf(stderr,"Format error near line %d while reading from %s, nread=%d\n", i+1,configFile,nread);
	    exit(1);
	}
    }
    nurl=MAXURLS;
}

int URLConfig::getIndex(const char *ident) {
    for (int i=0;i<nurl;i++)
	if (strcmp(ident,idents[i])==0)
	    return i;
    return -1;
}

const char *URLConfig::getHost(const char *ident) {	
    int index=getIndex(ident);
    if (index<0)
	return (const char *)0;
    return hosts[index];
}

int URLConfig::getPort(const char *ident) {	
    int index=getIndex(ident);
    if (index<0)
	return -1;
    return ports[index];
}
