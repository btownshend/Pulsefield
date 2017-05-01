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
    int frontendPort=urls.getPort("FE");
    const char *frontendHost=urls.getHost("FE");
    if (frontendPort==-1 || frontendHost==0) {
	fprintf(stderr,"Unable to locate FE in urlconfig.txt\n");
	frontend=0;
    } else {
	char cbuf[10];
	sprintf(cbuf,"%d",frontendPort);
	frontend = lo_address_new(frontendHost, cbuf);
	dbg("TrackerComm",1)  << "Set frontend to " << loutil_address_get_url(frontend) << std::endl;
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


void TrackerComm::sendMatrix(const char *path, int unit, const cv::Mat &m) const {
    assert(m.rows==3 && m.cols==3);
    if (lo_send(remote,path,"ifffffffff",unit,m.at<double>(0,0),m.at<double>(0,1),m.at<double>(0,2),m.at<double>(1,0),m.at<double>(1,1),m.at<double>(1,2),m.at<double>(2,0),m.at<double>(2,1),m.at<double>(2,2)) < 0) {
	dbg("TrackerComm.sendMatrix",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	    return;
    }
}

// Send transform that converts world coordinates into the camera view frame (without projecting)  3x4 matrix
// Consists of [R:T] where R is 3x3 rotation matrix and T is 3x1 translation
// Translation is applied after rotation
void TrackerComm::sendCameraView(int unit, const cv::Mat &rotMat, const cv::Mat &tvec) const {
    assert(rotMat.rows==3 && rotMat.cols==3);
    assert(tvec.rows==3 && tvec.cols==1);
    const char *path="/cal/cameraview";
    if (lo_send(remote,path,"iffffffffffff",unit,rotMat.at<double>(0,0),rotMat.at<double>(0,1),rotMat.at<double>(0,2),tvec.at<double>(0,0),rotMat.at<double>(1,0),rotMat.at<double>(1,1),rotMat.at<double>(1,2),tvec.at<double>(1,0), rotMat.at<double>(2,0),rotMat.at<double>(2,1),rotMat.at<double>(2,2),tvec.at<double>(2,0)) < 0) {
	dbg("TrackerComm.sendTransform",1) << "Failed send of " << path << " to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
}

// Set position of projector
// Although this may be redundant with the other information, calculating it is non-trivial
void TrackerComm::sendPose(int unit, const cv::Mat &pose) const {
    if (lo_send(remote,"/cal/pose","ifff",unit,pose.at<double>(0,0),pose.at<double>(0,1),pose.at<double>(0,2)) < 0 ) {
	dbg("TrackerComm.sendPose",1) << "Failed send of /cal/pose to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
}

// Send position and orientation of LIDAR
void TrackerComm::sendLIDARPose(int unit, const cv::Mat &pose, const cv::Mat &rvec) const {
    dbg("TrackerComm.sendLIDARPose",1) << "pose=" << pose << ", rvec=" << rvec << std::endl;
    if (lo_send(frontend,"/cal/lidarpose","ifff",unit,pose.at<double>(0,0),pose.at<double>(0,1),rvec.at<double>(0,0)) < 0 ) {
	dbg("TrackerComm.sendLIDARPose",1) << "Failed send of /cal/lidarpose to " << loutil_address_get_url(frontend) << ": " << lo_address_errstr(frontend) << std::endl;
	return;
    }
}

// Send projection (perspective map)
void TrackerComm::sendProjection(int unit, const cv::Mat &projection) const {
    if (lo_send(remote,"/cal/projection","iffffff",unit,projection.at<double>(0,0),projection.at<double>(0,1),projection.at<double>(0,2),
		projection.at<double>(1,0),projection.at<double>(1,1),projection.at<double>(1,2)) < 0 ) {
	dbg("TrackerComm.sendPose",1) << "Failed send of /cal/projection to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
}

void TrackerComm::sendMapping(int unit1, int unit2, int pnum, int numPoints, Point p1, Point p2) const {
    if (lo_send(remote,"/cal/mapping","iiiiffff",unit1,unit2,pnum,numPoints,p1.X(),p1.Y(),p2.X(),p2.Y()) < 0) {
	dbg("TrackerComm.sendMapping",1) << "Failed send of /cal/mapping to " << loutil_address_get_url(remote) << ": " << lo_address_errstr(remote) << std::endl;
	return;
    }
}
