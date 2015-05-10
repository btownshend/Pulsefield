#include "music.h"
#include "lo_util.h"
Music *Music::theInstance = NULL;


Music::Music() {
    bpm=120;
    setBeat(1,1); 
    remote=lo_address_new("192.168.0.29","7010");
}

int Music::handleOSCMessage(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Music.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;
    if (strcmp(types,"f")==0)
	send(path,argv[0]->f);
    else {
	dbg("Music.send",1) << "Unhandled type: " << types << std::endl;
	return -1;
    }
    return 0;
}

int Music::send(std::string path, float value) const {
    if (lo_send(remote,path.c_str(),"f",value) <0 ) {
	dbg("Music.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(100);
    return 0;
}

int Music::send(std::string path, std::string value) const {
    if (lo_send(remote,path.c_str(),"s",value.c_str()) <0 ) {
	dbg("Music.send",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return -1;
    }
    usleep(100);
    return 0;
}

void Music::frameTick(int frame) {
    if (frame % 10 == 0) {
	predict();
	TouchOSC::instance()->send("/ui/beat",std::to_string(getLastBar())+":"+std::to_string((int)getLastBeat()));
    }
    if (frame % 50 == 0)
	TouchOSC::instance()->send("/ui/tempo",std::to_string((int)getTempo()));
}

