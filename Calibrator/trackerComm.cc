#include <fstream>
#include <sys/time.h>
#include "lo_util.h"
#include "trackerComm.h"

TrackerComm *TrackerComm::theInstance=NULL;

TrackerComm::TrackerComm(URLConfig &urls)  {
    theInstance=this;
    int trackerPort=urls.getPort("VD");
    const char *trackerHost=urls.getHost("VD");
    if (trackerPort==-1 || trackerHost==0) {
	fprintf(stderr,"Unable to locate VD in urlconfig.txt\n");
	remote=0;
    } else {
	char cbuf[10];
	sprintf(cbuf,"%d",trackerPort);
	remote = lo_address_new(trackerHost, cbuf);
	dbg("TrackerComm",1)  << "Set remote to " << loutil_address_get_url(remote) << std::endl;
    }
}
		       
TrackerComm::~TrackerComm() {
    if (remote)
	lo_address_free(remote);
}

int TrackerComm::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("TrackerComm.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;

    std::string host=lo_address_get_hostname(lo_message_get_source(msg));
    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    const char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (handled) {
	;
    } else {
	dbg("TrackerComm.handleOSCMessage",1) << "Unhandled message: " << path << ": parse failed at token: " << tok << std::endl;
    }
    
    delete [] pathCopy;
    return handled?0:1;
}

void TrackerComm::sendCursors(const std::vector<Cursor> &cursors) const {
    for (int i=0;i<cursors.size();i++) {
	if (lo_send(remote,"/cal/cursor","iiiff",i,cursors.size(), cursors[i].unit, cursors[i].x, cursors[i].y) <0 ) {
	    dbg("TrackerComm.sendCursors",1) << "Failed send of /cal/cursor/proj to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return;
	}
    }
    if (cursors.size() == 0) {
	// Send message indicating 0 length
	if (lo_send(remote,"/cal/cursor","iiiff",-1,cursors.size(),0,0.0,0.0) <0 ) {
	    dbg("TrackerComm.sendCursors",1) << "Failed send of /cal/cursor/proj to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return;
	}
    }
}

void TrackerComm::sendTransform(int unit, bool inverse, const cv::Mat &hom) const {
    const char *path="/cal/transform";
    if (inverse)
	path="/cal/invtransform";
    if (lo_send(remote,path,"ifffffffff",unit,hom.at<double>(0,0),hom.at<double>(0,1),hom.at<double>(0,2),hom.at<double>(1,0),hom.at<double>(1,1),hom.at<double>(1,2),hom.at<double>(2,0),hom.at<double>(2,1),hom.at<double>(2,2))) {
	dbg("TrackerComm.sendTransform",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return;
    }
}

void TrackerComm::sendPose(int unit, const cv::Mat &pose) const {
    if (lo_send(remote,"/cal/pose","iffffffffffff",unit,pose.at<double>(0,0),pose.at<double>(0,1),pose.at<double>(0,2),pose.at<double>(0,3),
		pose.at<double>(1,0),pose.at<double>(1,1),pose.at<double>(1,2),pose.at<double>(1,3),
		pose.at<double>(2,0),pose.at<double>(2,1),pose.at<double>(2,2),pose.at<double>(2,3))) {
	dbg("TrackerComm.sendPose",1) << "Failed send of /cal/pose to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
}
