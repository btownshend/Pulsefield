#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include "urlconfig.h"

URLConfig::URLConfig(const char *configFile) {
    printf("Loading config from %s\n", configFile);
    filename=new char[strlen(configFile)+1];
    strcpy(filename,configFile);
    FILE *fd=fopen(configFile,"r");
    if (fd==NULL) {
	fprintf(stderr,"Unable to open url config file: %s\n", configFile);
	exit(1);
    }
    for (int i=0;i<MAXURLS;i++) {
	const unsigned int MAXLEN=2000;
	char tmp_ident[MAXLEN+1];
	char tmp_host[MAXLEN+1];
	while (true) {
	    int nread=fscanf(fd,"%50[^,],%50[^,],%d\n",tmp_ident,tmp_host,&ports[i]);
	    if (strlen(tmp_ident)>=MAXLEN || strlen(tmp_host)>=MAXLEN) {
		fprintf(stderr,"Data field too long in %s near line %d\n", configFile, i+1);
		exit(1);
	    }

	    if  (nread<0) {
		nurl=i;
		return;
	    }
	    if (nread==3)
		break;

	    fprintf(stderr,"Format error near line %d while reading from %s, nread=%d\n", i+1,configFile,nread);
	}
	idents[i]=new char[strlen(tmp_ident)+1];
	strcpy(idents[i],tmp_ident);
	hosts[i]=new char[strlen(tmp_host)+1];
	strcpy(hosts[i],tmp_host);

	printf("Set %s to %s:%d\n", idents[i], hosts[i], ports[i]);
    }
    nurl=MAXURLS;
}

URLConfig::~URLConfig() {
    // printf("Deleting URLConfig (nurl=%d)\n",nurl); fflush(stdout);
    delete [] filename;
    for (int i=0;i<nurl;i++) {
	delete [] idents[i];
	delete [] hosts[i];
    }
}

int URLConfig::getIndex(const char *ident) const {
    for (int i=0;i<nurl;i++)
	if (strcmp(ident,idents[i])==0)
	    return i;
    return -1;
}

const char *URLConfig::getHost(const char *ident) const {	
    int index=getIndex(ident);
    if (index<0)
	return (const char *)0;
    return hosts[index];
}

int URLConfig::getPort(const char *ident) const {	
    int index=getIndex(ident);
    if (index<0)
	return -1;
    return ports[index];
}
