#pragma once

#include <string>
#include "lo_util.h"
#include "dbg.h"

class Conductor {
    static const char *PORT;
    static Conductor *theInstance; // singleton
    lo_address remote;
    Conductor(std::string host) {
	remote=lo_address_new(host.c_str(),PORT);
    }
public:
    static Conductor *instance() {
	if (theInstance == NULL) {
	    theInstance=new Conductor("127.0.0.1");
	}
	return theInstance;
    }

    void setHostname(std::string host) {
	if (host != lo_address_get_hostname(remote)) {
	    lo_address_free(remote);
	    remote=lo_address_new(host.c_str(),PORT);
	    dbg("Conductor",1) << "Opening connection to " << loutil_address_get_url(remote) << std::endl;
	}
    }

    int send(std::string path, float value) const {
	dbg("Conductor.send",4) << "send " << path << "," << value << std::endl;
	if (lo_send(remote,path.c_str(),"f",value) <0 ) {
	    dbg("Conductor.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return -1;
	}
	return 0;
    }

    int send(std::string path, std::string value) const {
	dbg("Conductor.send",4) << "send " << path << "," << value << std::endl;
	if (lo_send(remote,path.c_str(),"s",value.c_str()) <0 ) {
	    dbg("Conductor.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return -1;
	}
	return 0;
    }

};

