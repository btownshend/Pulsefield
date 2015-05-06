#include "calibration.h"
#include "lasers.h"
#include "dbg.h"

static const int NUMRELATIVE=10;
static const int NUMABSOLUTE=4;

std::shared_ptr<Calibration> Calibration::theInstance;   // Singleton

static void send(std::string path, float value)  {
    TouchOSC::instance()->send(path,value);
}
static void send(std::string path, std::string value)  {
    TouchOSC::instance()->send(path,value);
}
static void send(std::string path, float val1, float val2)  {
    TouchOSC::instance()->send(path,val1,val2);
}

bool RelMapping::handleOSCMessage(char *tok, lo_arg **argv,int speed) {
    dbg("RelMapping",1) << "tok=" << tok << ", speed=" << speed << ", adjust=" << adjust << ", id=" << id << ", nunits=" << locked.size() <<  std::endl;
    bool handled=false;
    if (strcmp(tok,"adjust")==0) {
	int col=atoi(strtok(NULL,"/"))-1;
	//int row=atoi(strtok(NULL,"/"))-1;
	if (argv[0]->f>0)
	    adjust=col;
	else if (adjust==col)
	    adjust=-1;
	if (adjust>=devpt.size())
	    adjust=-1;
	handled=true;
    } else if (strcmp(tok,"dev")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"xy")==0) {
	    // xy pad
	    if (adjust>=0 && ~locked[adjust]) {
		devpt[adjust].setX(argv[0]->f);
		devpt[adjust].setY(argv[1]->f);
	    }
	    handled=true;
	}
    } else if (strcmp(tok,"lock")==0) {
	int col=atoi(strtok(NULL,"/"))-1;
	//	int row=atoi(strtok(NULL,"/"))-1;
	dbg("RelMapping",1) << "lock: col=" << col << ", size(locked)=" << locked.size() << std::endl;
	if (col<locked.size())
	    locked[col]=argv[0]->f > 0;
	handled=true;
    } else if (strcmp(tok,"left")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setX(std::max(-32767.0f,devpt[adjust].X()-speed));
	handled=true;
    } else if (strcmp(tok,"right")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setX(std::min(32767.0f,devpt[adjust].X()+speed));
	handled=true;
    } else if (strcmp(tok,"up")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setY(std::min(32767.0f,devpt[adjust].Y()+speed));
	handled=true;
    } else if (strcmp(tok,"down")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust].setY(std::max(-32767.0f,devpt[adjust].Y()-speed));
	handled=true;
    } else if (strcmp(tok,"center")==0) {
	if (adjust>=0 && ~locked[adjust] && argv[0]->f > 0)
	    devpt[adjust]=Point(0,0);
	handled=true;
    }
    
    return handled;
}


Calibration::Calibration(int _nunits): relMappings(NUMRELATIVE), absMappings(NUMABSOLUTE)  {
    nunits = _nunits;
    dbg("Calibration.Calibration",1) << "Constructing calibration with " << nunits << " units." << std::endl;
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]=std::shared_ptr<RelMapping>(new RelMapping(i,nunits));
    for (int i=0;i<absMappings.size();i++)
	absMappings[i]=std::shared_ptr<AbsMapping>(new AbsMapping(i,nunits));
    flipX=false;
    flipY=false;
    speed=100;
    absSelected=-1;
    relSelected=0;
    updateUI();
    showStatus("Initalized");
}

int Calibration::handleOSCMessage_impl(const char *path, const char *types, lo_arg **argv,int argc,lo_message msg) {
    dbg("Calibration.handleOSCMessage",1)  << "Got message: " << path << "(" << types << ") from " << loutil_address_get_url(lo_message_get_source(msg)) << std::endl;

    char *pathCopy=new char[strlen(path)+1];
    strcpy(pathCopy,path);
    char *tok=strtok(pathCopy,"/");

    bool handled=false;
    if (strcmp(tok,"cal")==0) {
	tok=strtok(NULL,"/");
	if (strcmp(tok,"rel")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"sel")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		assert(col==0);
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("Calibration",1) << "Selected relative position " << row << " with value " << argv[0]->f <<  std::endl;
		if (argv[0]->f > 0) {
		    relSelected=row;
		    if (absSelected>=0) {
			send("/cal/abs/sel/1/"+std::to_string(absSelected+1),0.0);
			absSelected=-1;
		    }
		    showStatus("Adjusting relative anchor point "+std::to_string(row));
		}
		handled=true;
	    }
	} else if (strcmp(tok,"abs")==0) {
	    tok=strtok(NULL,"/");
	    if (strcmp(tok,"sel")==0) {
		int col=atoi(strtok(NULL,"/"))-1;
		assert(col==0);
		int row=atoi(strtok(NULL,"/"))-1;
		dbg("Calibration",1) << "Selected absolute position " << row << " with value " << argv[0]->f <<  std::endl;
		if (argv[0]->f > 0) {
		    absSelected=row;
		    if (relSelected>=0) {
			send("/cal/rel/sel/1/"+std::to_string(relSelected+1),0.0);
			relSelected=-1;
		    }
		    showStatus("Adjusting absolute anchor point "+std::to_string(row));
		}
		handled=true;
	    }
	} else if (strcmp(tok,"speed")==0) {
	    speed=argv[0]->f;
	    showStatus("Set speed to " + std::to_string(std::round(speed)));
	    handled=true;
	}  else if (strcmp(tok,"flipx")==0) {
	    flipX=argv[0]->f>0;
	    handled=true;
	}  else if (strcmp(tok,"flipy")==0) {
	    flipY=argv[0]->f>0;
	    handled=true;
	} else if (strcmp(tok,"recompute")==0) {
	    if (argv[0]->f > 0) {
		if (recompute() == 0)
		    showStatus("Updated mappings");
		else
		    showStatus("recompute() failed");
	    }
	    handled=true;
	} else if (strcmp(tok,"save")==0) {
	    Lasers::instance()->save();
	    handled=true;
	} else if (strcmp(tok,"load")==0) {
	    Lasers::instance()->load();
	    handled=true;
	}  else if (relSelected>=0) {
	    handled=relMappings[relSelected]->handleOSCMessage(tok,argv,speed);
	} else if (absSelected>=0) {
	    handled=absMappings[absSelected]->handleOSCMessage(tok,argv,speed);
	}
    }
    updateUI();
    delete pathCopy;
    return handled?0:1;
}

void Calibration::updateUI() const {
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]->updateUI(i==relSelected);
    for (int i=0;i<absMappings.size();i++)
	absMappings[i]->updateUI(i==absSelected);

    send("/cal/flipx",flipX?1.0:0.0);
    send("/cal/flipy",flipY?1.0:0.0);
    send("/cal/speed",speed);
}

void RelMapping::updateUI(bool selected) const {
    if (selected) {
	if (id>=0)
	    send("/cal/rel/sel/1/"+std::to_string(id+1),1.0);
	for (int i=0;i<locked.size();i++) {
	    send("/cal/lock/"+std::to_string(i+1)+"/1",locked[i]?1.0:0.0);
	    send("/cal/status/"+std::to_string(i+1)+"/"+std::to_string(id+1),locked[i]?1.0:0.0);
	    send("/cal/adjust/"+std::to_string(i+1)+"/1",(i==adjust)?1.0:0.0);
	    if (adjust<0) {
		send("/cal/dev/x","");
		send("/cal/dev/y","");
	    } else {
		send("/cal/dev/x",std::to_string((int)std::round(devpt[adjust].X())));
		send("/cal/dev/y",std::to_string((int)std::round(devpt[adjust].Y()+0.5)));
		send("/cal/dev/xy",devpt[adjust].X(),devpt[adjust].Y());
	    }
	}
    }
}

void AbsMapping::updateUI(bool selected) const {
    // Retrieive fiducial location
    ((AbsMapping *)this)->floor=Lasers::instance()->getLaser(0)->getTransform().getFloorPoint(id);
    RelMapping::updateUI(selected);
    if (selected)
	send("/cal/abs/sel/1/"+std::to_string(id+1),1.0);
    send("/cal/abs/lock/"+std::to_string(id+1)+"/1",glocked?1.0:0.0);
    send("/cal/abs/"+std::to_string(id+1)+"/x",std::round(floor.X()*100)/100);
    send("/cal/abs/"+std::to_string(id+1)+"/y",std::round(floor.Y()*100)/100);
}

// Display status in touchOSC
void Calibration::showStatus(std::string line1,std::string line2, std::string line3) const {
    send("/cal/status/1",line1);
    send("/cal/status/2",line2);
    send("/cal/status/3",line3);
}

void AbsMapping::save(ptree &p) const {
    p.put("id",id);
    p.put("floor.x",floor.X());
    p.put("floor.y",floor.Y());
    RelMapping::save(p);
}

void AbsMapping::load(ptree &p)  {
    id=p.get("id",id);
    floor=Point(p.get("floor.x",floor.X()),p.get("floor.y",floor.Y()));
    RelMapping::load(p);
}

void RelMapping::save(ptree &p) const {
    dbg("RelMapping.save",1) << "Saving relMapping " << id << " to ptree" << std::endl;
    if (id>=0)
	p.put("id",id);
    p.put("adjust",adjust);
    ptree dpts;
    for (int i=0;i<devpt.size();i++) {
	ptree dp;
	dp.put("x",devpt[i].X());
	dp.put("y",devpt[i].Y());
	dp.put("locked",locked[i]);
	dpts.push_back(std::make_pair("",dp));
    }
    p.put_child("devpts",dpts);
}

void RelMapping::load(ptree &p) {
    dbg("RelMapping.save",1) << "Loading relMapping from ptree" << std::endl;
    id=p.get("id",-1);
    adjust=p.get("adjust",adjust);

    try {
	ptree dp=p.get_child("devpts");
	int i=0;
	for (ptree::iterator v = dp.begin(); v != dp.end();++v) {
	    if (i>=devpt.size())
		break;
	    ptree val=v->second;
	    devpt[i]=Point(val.get<double>("x",devpt[i].X()),val.get<double>("y",devpt[i].Y()));
	    locked[i]=val.get<bool>("locked",locked[i]);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'devpts' in laser settings" << std::endl;
    }
}

void Calibration::save(ptree &p) const {
    dbg("Calibration.save",1) << "Saving calibration to ptree" << std::endl;
    ptree rm;
    for (unsigned int i=0;i<relMappings.size();i++)  {
	ptree mapping;
	relMappings[i]->save(mapping);
	rm.push_back(std::make_pair("",mapping));
    }
    p.put_child("rel",rm);
    ptree am;
    for (unsigned int i=0;i<absMappings.size();i++)  {
	ptree mapping;
	absMappings[i]->save(mapping);
	am.push_back(std::make_pair("",mapping));
    }
    p.put_child("abs",am);
    p.put("speed",speed);
    p.put("flipX",flipX);
    p.put("flipY",flipY);
    showStatus("Saved configuration");
}

void Calibration::load(ptree &p) {
    dbg("Calibration.load",1) << "Loading transform from ptree" << std::endl;
    speed=p.get("speed",speed);
    flipX=p.get("flipX",flipX);
    flipY=p.get("flipY",flipY);
    ptree rm;
    try {
	rm=p.get_child("rel");
	int i=0;
	for (ptree::iterator v = rm.begin(); v != rm.end();++v) {
	    if (i>=relMappings.size())
		break;
	    relMappings[i]->load(v->second);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'rel' in laser settings" << std::endl;
    }
    ptree am;
    try {
	am=p.get_child("abs");
	int i=0;
	for (ptree::iterator v = am.begin(); v != am.end();++v) {
	    if (i>=absMappings.size())
		break;
	    absMappings[i]->load(v->second);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Uable to find 'abs' in laser settings" << std::endl;
    }
    showStatus("Loaded configuration");
}

// Add these mapping(s) to the lists of features and pairwiseMatches
// features vector has one entry for each laser (N)
// matches has one entry for every permutation (i.e. N^2)
int RelMapping::addMatches(std::vector<cv::detail::ImageFeatures> &features,    std::vector<cv::detail::MatchesInfo> &pairwiseMatches) const {
    int numAdded=0;
    int nunits=locked.size();
    for (int i=0;i<nunits;i++) {
	if (!locked[i]) continue;
	Point flat1=Lasers::instance()->getLaser(i)->getTransform().deviceToFlat(devpt[i]);
	for (int j=i+1;j<nunits;j++) {
	    if (!locked[j]) continue;
	    Point flat2=Lasers::instance()->getLaser(j)->getTransform().deviceToFlat(devpt[j]);
	    dbg("RelMapping.addMatches",1) << "Adding pair from relMapping " << id << ": laser " << i << "@" << flat1 << " <->  laser " << j << "@" << flat2 << std::endl;
	    cv::KeyPoint kp1,kp2;
	    kp1.pt.x=flat1.X()+1;
	    kp1.pt.y=flat1.Y()+1;
	    features[i].keypoints.push_back(kp1);

	    kp2.pt.x=flat2.X()+1;
	    kp2.pt.y=flat2.Y()+1;
	    features[j].keypoints.push_back(kp2);

	    cv::DMatch m;   // Declared in features2d/features2d.hpp
	    m.queryIdx=features[i].keypoints.size()-1;
	    m.trainIdx=features[j].keypoints.size()-1;
	    m.distance=0;	// Probably usually set later as a function of how the match works
	    m.imgIdx=j; // Unsure... appears to not be used, but in any case it seems that it is the train image index
	    pairwiseMatches[i*nunits+j].inliers_mask.push_back(1);	// This match is an "inlier"
	    pairwiseMatches[i*nunits+j].matches.push_back(m);
	    pairwiseMatches[i*nunits+j].num_inliers++;
	    numAdded++;
	}
    }
    return numAdded;
}

std::ostream &flatMat(std::ostream &s, const cv::Mat &m) {
    s << "[";
    cv::Size sz=m.size();
    for (int i=0;i<sz.width;i++) {
	for (int j=0;j<sz.height;j++)
	    s << m.at<double>(i,j) << " ";
	if (i==sz.width-1)
	    s << "]";
    }
    return s;
}

std::ostream &operator<<(std::ostream &s, const cv::detail::CameraParams &c) {
    // For some reason, outputting the R matrix causes output to hang
    s << "f=" << c.focal << ", aspect=" << c.aspect << ", pp=[" << c.ppx << "," << c.ppy << "] , R=";
    flatMat(s,c.R);
    s << ", t=";
    flatMat(s,c.t) ;
    return s;
}

// Compute all the homographies using openCV
// see OpenCV motion_estimators.cpp for use (documentation doesn't cut it)
int Calibration::recompute() {
    std::vector<cv::detail::ImageFeatures> features(nunits);
    // One feature entry for each image
    for (int i=0;i<nunits;i++) {
	features[i].img_idx=i;
	features[i].img_size.width=2;
	features[i].img_size.height=2;
    }
    std::vector<cv::detail::MatchesInfo> pairwiseMatches(nunits*nunits);
    // One pairwiseMatches entry for each combination
    for (int i=0;i<nunits;i++) {
	for (int j=0;j<nunits;j++) {
	    pairwiseMatches[i*nunits+j].src_img_idx=i;
	    pairwiseMatches[i*nunits+j].dst_img_idx=j;
	    pairwiseMatches[i*nunits+j].confidence=1.0;
	}
    }
    std::vector<cv::detail::CameraParams>  cameras(nunits);
    
    for (int i=0;i<nunits;i++) {
	dbg("Calibration.recompute",1) << "Camera " << i << ": " << cameras[i] << std::endl;
    }
    
    // Build features, matches vectors
    int numMatches = 0;
    for (int i=0;i<relMappings.size();i++)
	numMatches+=relMappings[i]->addMatches(features, pairwiseMatches);
    dbg("Calibration.recompute",1) << "Have " << features.size() << " features with " << numMatches << " pairwise matches." << std::endl;
    
    // Estimate camera parameters roughly
    cv::detail::HomographyBasedEstimator estimator(true);
    try {
	estimator(features, pairwiseMatches, cameras);
    } catch (cv::Exception &e) {
	dbg("Calibration.recompute",1) << "OpenCV.estimator failed: " << e.what() << std::endl;
	return -1;
    }
    for (size_t i = 0; i < cameras.size(); ++i) {
	dbg("Calibration.recompute",1) << "After homography estimation, camera " << i << ": " << cameras[i] << std::endl;
    }

    // Homography estimator returns R as CV_64F, but bundle adjuster expects CV_32F
    for (size_t i = 0; i < cameras.size(); ++i) {
	cv::Mat R;
        cameras[i].R.convertTo(R, CV_32F);
        cameras[i].R = R;
    }

    // 5- Refine camera parameters globally
    cv::detail::BundleAdjusterReproj adjuster;
    adjuster.setConfThresh(0.0);
    try {
	adjuster(features, pairwiseMatches, cameras);
    } catch (cv::Exception &e) {
	dbg("Calibration.recompute",1) << "OpenCV.estimator failed: " << e.what() << std::endl;
	return -1;
    }

    for (size_t i = 0; i < cameras.size(); ++i)     {
	dbg("Calibration.recompute",1) << "After bundle adjustment, camera " << i << ": " << cameras[i] << std::endl;
    }

    for (int i=0;i<pairwiseMatches.size();i++) {
	cv::detail::MatchesInfo p = pairwiseMatches[i];
	cv::detail::CameraParams c1=cameras[p.src_img_idx];
	cv::detail::CameraParams c2=cameras[p.dst_img_idx];
	
	cv::Mat_<float> K1 = cv::Mat::eye(3, 3, CV_32F);
	K1(0,0) = c1.focal; K1(0,2) = c1.ppx;
	K1(1,1) = c1.focal*c1.aspect; K1(1,2) = c1.ppy;
	
	cv::Mat_<float> K2 = cv::Mat::eye(3, 3, CV_32F);
	K2(0,0) = c2.focal; K2(0,2) = c2.ppx;
	K2(1,1) = c2.focal*c2.aspect; K2(1,2) = c2.ppy;

	cv::Mat_<float> H = K2;
	H=H* c2.R.inv();
	H=H* c1.R;
	H=H* K1.inv();


	for (int j=0;j<p.matches.size();j++) {
	    cv::DMatch m=p.matches[j];
	    cv::Point2f p1 = features[p.src_img_idx].keypoints[m.queryIdx].pt;
	    cv::Point2f p2 = features[p.dst_img_idx].keypoints[m.trainIdx].pt;
            double x = H(0,0)*p1.x + H(0,1)*p1.y + H(0,2);
            double y = H(1,0)*p1.x + H(1,1)*p1.y + H(1,2);
            double z = H(2,0)*p1.x + H(2,1)*p1.y + H(2,2);
	    double ex=p2.x-x/z;
	    double ey=p2.y-y/z;
	    dbg("Calibration.recompute",1) << p.src_img_idx << "-" << p.dst_img_idx  << " e=[" << ex << "," << ey  << "]" << std::endl;
	}
    }
    return 0;
}
