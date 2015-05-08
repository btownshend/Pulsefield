#include "calibration.h"
#include "lasers.h"
#include "dbg.h"
#include "opencv2/calib3d/calib3d.hpp"

static const float WORLDSIZE=30;   // Maximum extents of world (in meters)

std::shared_ptr<Calibration> Calibration::theInstance=NULL;   // Singleton

static void send(std::string path, float value)  {
    TouchOSC::instance()->send(path,value);
    dbg("Calibration.send",3) << "send(" << path << "," << value << ")" << std::endl;
}
static void send(std::string path, std::string value)  {
    TouchOSC::instance()->send(path,value);
    dbg("Calibration.send",3) << "send(" << path << ",\"" << value << "\")" << std::endl;
}
static void send(std::string path, float val1, float val2)  {
    TouchOSC::instance()->send(path,val1,val2);
    dbg("Calibration.send",2) << "send(" << path << "," << val1 << "," << val2 << ")" << std::endl;
}

// Convert from logarithmic speed (~1/32767 - 0.1) to [0,1] rotary setting
static float speedToRotary(float speed) {
    return log10(speed*32767)/log10(1000);
}

// And back
static float rotaryToSpeed(float rotary) {
    return pow(10,rotary*log10(1000))/32767;
}

bool RelMapping::handleOSCMessage(std::string tok, lo_arg **argv,float speed,bool flipX1, bool flipY1, bool flipX2, bool flipY2) {
    dbg("RelMapping",1) << "tok=" << tok << ", speed=" << speed << std::endl;
    bool handled=false;
    
    Point *pt=&pt1[selected];
    char *nexttok=strtok(NULL,"/");
    if (nexttok){
	dbg("RelMapping",1) << "tok=" << tok << ", nexttok=" << nexttok << ", speed=" << speed << std::endl;
    } else
	dbg("RelMapping",1) << "tok=" << tok << ", speed=" << speed << std::endl;
    float speedX=speed*(flipX1?-1:1);
    float speedY=speed*(flipY1?-1:1);;
    if (nexttok && strcmp(nexttok,"2")==0) {
	dbg("RelMapping",2) << "Using pt2" << std::endl;
	pt=&pt2[selected];
	speedX=speed*(flipX2?-1:1);
	speedY=speed*(flipY2?-1:1);;
    }

    if (tok=="xy") {
	// xy pad
	if (~locked[selected]) {
	    pt->setX(argv[0]->f*(speedX>0?1:-1));
	    pt->setY(argv[1]->f*(speedY>0?1:-1));
	}
	handled=true;
    } else if (tok=="lock") {
	locked[selected]=argv[0]->f > 0;
	handled=true;
    } else if (tok=="selpair") {
	if (argv[0]->f > 0)
	    selected=atoi(nexttok)-1;
	handled=true;
    } else if (tok=="left") {
	if (~locked[selected] && argv[0]->f > 0)
	    pt->setX(std::max(-1.0f,pt->X()-speedX));
	handled=true;
    } else if (tok=="right") {
	if (selected>=0 && ~locked[selected] && argv[0]->f > 0)
	    pt->setX(std::min(1.0f,pt->X()+speedX));
	handled=true;
    } else if (tok=="up") {
	if (selected>=0 && ~locked[selected] && argv[0]->f > 0)
	    pt->setY(std::min(1.0f,pt->Y()+speedY));
	handled=true;
    } else if (tok=="down") {
	if (selected>=0 && ~locked[selected] && argv[0]->f > 0)
	    pt->setY(std::max(-1.0f,pt->Y()-speedY));
	handled=true;
    }
    
    return handled;
}

// Get coordinate of pt[i] in device [-32767,32767] or world ([-WORLDSIZE,WORLDSIZE],[0,WORLDSIZE])
Point RelMapping::getDevicePt(int i,int which,bool doRound) const {
    Point res;
    if (which==-1)
	which=selected;
    if (i==0)
	res=pt1[which]*32767;
    else if (isWorld) {
	res=Point(pt2[which].X()*WORLDSIZE,(pt2[which].Y()+1)*WORLDSIZE/2);
	if (doRound)
	    res=Point(std::round(res.X()*100)/100,std::round(res.Y()*100)/100);
	return res;
    } else
	res=pt2[which]*32767;
    if (doRound)
	res=Point(std::round(res.X()),std::round(res.Y()));
    return res;
}
       
// Set coordinate of pt[i] in device [-32767,32767] or world ([-WORLDSIZE,WORLDSIZE],[0,WORLDSIZE])
void  RelMapping::setDevicePt(Point p, int i,int which)  {
    dbg("RelMapping.setDevicePt",2) <<"setDevicePt(" << p << "," << i << "," << which << ")" << std::endl;
    Point res;
    if (which==-1)
	which=selected;
    if (i==0)
	pt1[which]=p/32767;
    else if (isWorld) {
	pt2[which]=Point(p.X()/WORLDSIZE,p.Y()*2/WORLDSIZE-1);
    } else
	pt2[which]=p/32767;
}
       
Calibration::Calibration(int _nunits): homographies(_nunits+1) {
    nunits = _nunits;
    dbg("Calibration.Calibration",1) << "Constructing calibration with " << nunits << " units." << std::endl;
    assert(nunits>0);
    for (int i=0;i<homographies.size();i++)
	homographies[i]=cv::Mat::eye(3, 3, CV_32F);	// Initialize to identity matrix

    for (int i=0;i<nunits;i++)
	for (int j=i+1;j<nunits+1;j++) {
	    relMappings.push_back(std::shared_ptr<RelMapping>(new RelMapping(i,j,j==nunits)));
	}
    for (int i=0;i<nunits;i++) {
	flipX.push_back(false);
	flipY.push_back(false);
    }
    curMap=relMappings[0];
    speed=0.05;
    laserMode=CM_NORMAL;
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
	if (strcmp(tok,"sel")==0) {
	    if (argv[0]->f > 0) {
		std::string dir=strtok(NULL,"/");
		int which=atoi(strtok(NULL,"/"))-1;
		int cur[2];
		cur[0]=curMap->getUnit(0);
		cur[1]=curMap->getUnit(1);
		if (dir=="left") {
		    if (cur[which]>0)
			cur[which]--;
		} else {
		    if (cur[which]<nunits-1 || (which == 1 && cur[which]==nunits-1))
			cur[which]++;
		}
		dbg("Calibration",1) << "Selected " << cur[0] << ", " << cur[1] << std::endl;
		for (int i=0;i<relMappings.size();i++) {
		    std::shared_ptr<RelMapping> rm=relMappings[i];
		    if (rm->getUnit(0)==cur[0] && rm->getUnit(1)==cur[1]) {
			curMap=rm;
			dbg("Calibration",1) << "Change curMap to " << curMap->getUnit(0) << "-" << curMap->getUnit(1) << std::endl;
			break;
		    }
		}
	    }
	    handled=true;
	} else if (strcmp(tok,"speed")==0) {
	    speed=rotaryToSpeed(argv[0]->f);
	    showStatus("Set speed to " + std::to_string(std::round(speed*32767)));
	    handled=true;
	}  else if (strcmp(tok,"flipx")==0) {
	    int which=atoi(strtok(NULL,"/"))-1;
	    if (which==0)
		flipX[curMap->getUnit(0)]=argv[0]->f>0;
	    else
		flipX[curMap->getUnit(1)]=argv[0]->f>0;
	    handled=true;
	}  else if (strcmp(tok,"flipy")==0) {
	    int which=atoi(strtok(NULL,"/"))-1;
	    if (which==0)
		flipY[curMap->getUnit(0)]=argv[0]->f>0;
	    else
		flipY[curMap->getUnit(1)]=argv[0]->f>0;
	    handled=true;
	} else if (strcmp(tok,"recompute")==0) {
	    if (argv[0]->f > 0) {
		if (recompute() == 0)
		    showStatus("Updated mappings");
	    }
	    handled=true;
	} else if (strcmp(tok,"save")==0) {
	    Lasers::instance()->save();
	    handled=true;
	} else if (strcmp(tok,"load")==0) {
	    Lasers::instance()->load();
	    handled=true;
	} else if (strcmp(tok,"lasermode")==0) {
	    if (argv[0]->f > 0) {
		int col=atoi(strtok(NULL,"/"))-1;
		laserMode=(LaserMode)(col);
		Lasers::instance()->setFlag("calibration",laserMode!=CM_NORMAL);
	    }
	    handled=true;
	}  else {
	    // Pass down to currently selected relMapping
	    dbg("Calibration.handleOSCMessage",1) << "Handing off message to curMap" << std::endl;
	    handled=curMap->handleOSCMessage(tok,argv,speed,flipX[curMap->getUnit(0)],flipY[curMap->getUnit(0)],flipX[curMap->getUnit(1)],flipY[curMap->getUnit(1)]);
	}
    }
    updateUI();
    delete pathCopy;
    return handled?0:1;
}

void Calibration::updateUI() const {
    curMap->updateUI(flipX[curMap->getUnit(0)],flipY[curMap->getUnit(0)],flipX[curMap->getUnit(1)],flipY[curMap->getUnit(1)]);
    
    send("/cal/speed",speedToRotary(speed));
    send("/cal/lasermode/"+std::to_string((int)laserMode+1)+"/1",1.0);
    for (int i=0;i<relMappings.size();i++)
	relMappings[i]->sendCnt();
}

void RelMapping::sendCnt() const {
    int cnt=0;
    for (int i=0;i<locked.size();i++)
	if (locked[i]) cnt++;
    
    std::string cstr=std::to_string(cnt);
    if (cnt==0)
	cstr="";
    if (isWorld)
	send("/cal/cnt/"+std::to_string(unit1+1)+"/W",cstr);
    else
	send("/cal/cnt/"+std::to_string(unit1+1)+"/"+std::to_string(unit2+1),cstr);
}

void RelMapping::updateUI(bool flipX1,bool flipY1, bool flipX2, bool flipY2) const {
    dbg("RelMapping",1) << "updateUI for " << unit1 << "-" << unit2 << std::endl;
    send("/cal/sel/val/1",std::to_string(unit1+1));
    if (isWorld)
	send("/cal/sel/val/2","W");
    else
	send("/cal/sel/val/2",std::to_string(unit2+1));
    
    send("/cal/lock",locked[selected]?1.0:0.0);
    for (int i=0;i<locked.size();i++)
	send("/cal/led/"+std::to_string(i+1),locked[i]?1.0:0.0);
    send("/cal/selpair/"+std::to_string(selected+1)+"/1",1.0);
    send("/cal/flipx/1",flipX1?1.0:0.0);
    send("/cal/flipy/1",flipY1?1.0:0.0);
    send("/cal/flipx/2",flipX2?1.0:0.0);
    send("/cal/flipy/2",flipY2?1.0:0.0);

    send("/cal/xy/1",pt1[selected].X()*(flipX1?-1:1),pt1[selected].Y()*(flipY1?-1:1));
    send("/cal/x/1",getDevicePt(0,-1,true).X());
    send("/cal/y/1",getDevicePt(0,-1,true).Y());
    send("/cal/xy/2",pt2[selected].X()*(flipX2?-1:1),pt2[selected].Y()*(flipY2?-1:1));
    send("/cal/x/2",getDevicePt(1,-1,true).X());
    send("/cal/y/2",getDevicePt(1,-1,true).Y());
    std::vector<float> error=updateErrors();
    for (int i=0;i<error.size();i++)
	if (isnan(error[i]))
	    send("/cal/error/"+std::to_string(i+1),"");
	else if (error[i] >= 100)
	    send("/cal/error/"+std::to_string(i+1),std::round(error[i]));
	else if (error[i] >= 10)
	    send("/cal/error/"+std::to_string(i+1),std::round(error[i]*10)/10);
	else
	    send("/cal/error/"+std::to_string(i+1),std::round(error[i]*100)/100);
}

// Display status in touchOSC
void Calibration::showStatus(std::string line1,std::string line2, std::string line3) const {
    send("/cal/status/1",line1);
    send("/cal/status/2",line2);
    send("/cal/status/3",line3);
    dbg("Calibration.showStatus",1) << line1 << "; " << line2 << "; " << line3 << std::endl;
}

void RelMapping::save(ptree &p) const {
    dbg("RelMapping.save",1) << "Saving relMapping " << unit1 << "-" << unit2 << " to ptree" << std::endl;
    p.put("unit1",unit1);
    p.put("unit2",unit2);
    p.put("isWorld",isWorld);
    p.put("selected",selected);
    ptree pairs;
    for (int i=0;i<pt1.size();i++) {
	ptree dp;
	dp.put("pt1.x",pt1[i].X());
	dp.put("pt1.y",pt1[i].Y());
	dp.put("pt2.x",pt2[i].X());
	dp.put("pt2.y",pt2[i].Y());
	dp.put("locked",locked[i]);
	pairs.push_back(std::make_pair("",dp));
    }
    p.put_child("pairs",pairs);
}

void RelMapping::load(ptree &p) {
    dbg("RelMapping.save",1) << "Loading relMapping from ptree" << std::endl;
    unit1=p.get("unit1",0);
    unit2=p.get("unit2",1);
    selected=p.get("selected",selected);

    try {
	ptree dp=p.get_child("pairs");
	int i=0;
	for (ptree::iterator v = dp.begin(); v != dp.end();++v) {
	    if (i>=pt1.size())
		break;
	    ptree val=v->second;
	    pt1[i]=Point(val.get<double>("pt1.x",pt1[i].X()),val.get<double>("pt1.y",pt1[i].Y()));
	    pt2[i]=Point(val.get<double>("pt2.x",pt2[i].X()),val.get<double>("pt2.y",pt2[i].Y()));
	    locked[i]=val.get<bool>("locked",locked[i]);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Unable to find 'pairs' in laser settings" << std::endl;
    }
}

void Calibration::save(ptree &p) const {
    dbg("Calibration.save",1) << "Saving calibration to ptree" << std::endl;
    p.put("nunits",nunits);
    ptree rm;
    for (unsigned int i=0;i<relMappings.size();i++)  {
	ptree mapping;
	relMappings[i]->save(mapping);
	rm.push_back(std::make_pair("",mapping));
    }
    p.put_child("mappings",rm);
    p.put("speed",speed);
    ptree flips;
    for (int i=0;i<flipX.size();i++) {
	ptree f;
	f.put("flipX",flipX[i]);
	f.put("flipY",flipY[i]);
	flips.push_back(std::make_pair("",f));
    }
    p.put_child("flips",flips);
    showStatus("Saved configuration");
}

void Calibration::load(ptree &p) {
    dbg("Calibration.load",1) << "Loading transform from ptree" << std::endl;
    int ldunits=p.get("nunits",nunits);
    if (ldunits!=nunits) {
	dbg("Calibration.load",1) << "Loading file with " << ldunits << " into program setup with " << nunits << " units." << std::endl;
    }
    speed=p.get("speed",speed);
    ptree f;
    f=p.get_child("flips");
    int i=0;
    for (ptree::iterator v = f.begin(); v != f.end();++v) {
	if (i>=flipX.size())
	    break;
	ptree val=v->second;
	flipX[i]=val.get<bool>("flipX",flipX[i]);
	flipY[i]=val.get<bool>("flipY",flipY[i]);
	i++;
    }
    
    ptree rm;
    try {
	rm=p.get_child("mappings");
	int i=0;
	for (ptree::iterator v = rm.begin(); v != rm.end();++v) {
	    if (i>=relMappings.size())
		break;
	    relMappings[i]->load(v->second);
	    i++;
	}
    } catch (boost::property_tree::ptree_bad_path ex) {
	std::cerr << "Unable to find 'mappings' in laser settings" << std::endl;
    }
    showStatus("Loaded configuration");
}

// Add these mapping(s) to the lists of features and pairwiseMatches
// features vector has one entry for each laser (N)
// matches has one entry for every permutation (i.e. N^2)
// matches are entered with lowest number unit as src
// note: queryIdx refers to srcImg,  trainIdx refers to dstImg
int RelMapping::addMatches(std::vector<cv::detail::ImageFeatures> &features,    std::vector<cv::detail::MatchesInfo> &pairwiseMatches) const {
    int numAdded=0;
    for (int i=0;i<pairwiseMatches.size();i++) {
	cv::detail::MatchesInfo &pm = pairwiseMatches[i];
	if (pm.src_img_idx==unit1 && pm.dst_img_idx==unit2) {
	    for (int j=0;j<pt1.size();j++) {
		if (!locked[j])
		    continue;
		Point flat1=Lasers::instance()->getLaser(pm.src_img_idx)->getTransform().deviceToFlat(getDevicePt(0,j));
		Point flat2=Lasers::instance()->getLaser(pm.dst_img_idx)->getTransform().deviceToFlat(getDevicePt(1,j));
		dbg("RelMapping.addMatches",1) << "Adding pair from relMapping of laser " <<pm.src_img_idx << "@" << flat1 << " <->  laser " <<pm.dst_img_idx << "@" << flat2 << std::endl;
		cv::KeyPoint kp1,kp2;
		kp1.pt.x=flat1.X();
		kp1.pt.y=flat1.Y();
		features[pm.src_img_idx].keypoints.push_back(kp1);

		kp2.pt.x=flat2.X();
		kp2.pt.y=flat2.Y();
		features[pm.dst_img_idx].keypoints.push_back(kp2);

		cv::DMatch m;   // Declared in features2d/features2d.hpp
		m.queryIdx=features[pm.src_img_idx].keypoints.size()-1;
		m.trainIdx=features[pm.dst_img_idx].keypoints.size()-1;
		dbg("RelMapping.addMatches",2) << pm.dst_img_idx << "." << m.trainIdx << std::endl;
		m.distance=0;	// Probably usually set later as a function of how the match works
		m.imgIdx=pm.dst_img_idx; // Unsure... appears to not be used, but in any case it seems that it is the train image index
		pm.inliers_mask.push_back(1);	// This match is an "inlier"
		pm.matches.push_back(m);
		pm.num_inliers++;
		numAdded++;
	    }
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
	s << std::endl;
	if (i==sz.width-1)
	    s << "]";
    }
    return s;
}

// Compute all the homographies using openCV
// see OpenCV motion_estimators.cpp for use (documentation doesn't cut it)
int Calibration::recompute() {
    std::vector<cv::detail::ImageFeatures> features(nunits+1);
    // One feature entry for each image
    for (int i=0;i<nunits+1;i++) {
	features[i].img_idx=i;
    }
    std::vector<cv::detail::MatchesInfo> pairwiseMatches;
    // One pairwiseMatches entry for each combination
    for (int i=0;i<nunits+1;i++) {
	for (int j=0;j<nunits+1;j++) {
	    if (i!=j) {
		cv::detail::MatchesInfo pm;
		pm.src_img_idx=i;
		pm.dst_img_idx=j;
		pairwiseMatches.push_back(pm);
	    }
	}
    }
    
    // Build features, matches vectors
    int numMatches = 0;
    for (int i=0;i<relMappings.size();i++)
	numMatches+=relMappings[i]->addMatches(features, pairwiseMatches);
    dbg("Calibration.recompute",1) << "Have " << features.size() << " features with " << numMatches << " pairwise matches." << std::endl;

    // Build matrix of linkage counts
    cv::Mat_<int> linkages = cv::Mat::zeros(nunits+1,nunits+1,CV_32S);
    std::vector<int> total(nunits+1);
    for (int i=0;i<pairwiseMatches.size();i++) {
	cv::detail::MatchesInfo pm = pairwiseMatches[i];
	if (pm.src_img_idx != pm.dst_img_idx) {
	    int n=pm.matches.size();
	    linkages[pm.src_img_idx][pm.dst_img_idx]+=n;
	    linkages[pm.dst_img_idx][pm.src_img_idx]+=n;
	    total[pm.src_img_idx]+=n;
	}
    }
    // Find best starting image -- has most pairs with some other image and, in case of a tie, has the most total connections
    int bestcnt=0;
    int refLaser=0;
    for (int i=0;i<pairwiseMatches.size();i++) {
	cv::detail::MatchesInfo pm = pairwiseMatches[i];
	if (pm.matches.size() >= bestcnt) {
	    if (pm.matches.size()>bestcnt || total[pm.src_img_idx] > total[refLaser]) {
		refLaser = pm.src_img_idx;
		bestcnt=pm.matches.size();
	    }
	}
    }
    dbg("Calibration.recompute",1) << "Using laser " << refLaser << " as reference with " << bestcnt << " connections to another image and " << total[refLaser] << " total connections." << std::endl;
    homographies[refLaser]=cv::Mat::eye(3, 3, CV_32F);	// Use first laser as a reference for now

    std::vector<bool> found(nunits+1);	// Flag for whether a laser has been calibrated
    found[refLaser]=true;

    // Find homographies (nunits+1)-1 times
    int resultCode=0;
    for (int rep=0;rep<nunits;rep++) {
	// Match next unit with found ones

	// Count number of matches from each unit to set of found ones and select mostly highly connected one (lowest unit in case of ties)
	int bestcnt=-1;
	int curUnit=-1;
	for (int i=0;i<nunits+1;i++) {
	    if (!found[i]) {
		int nmatches=0;
		for (int j=0;j<nunits+1;j++)
		    if (found[j])
			nmatches+=linkages[i][j];
		if (nmatches>bestcnt) {
		    bestcnt=nmatches;
		    curUnit=i;
		}
	    }
	}
	dbg("Calibration.recompute",1) << "Computing linkage to laser " << curUnit <<  " with " << bestcnt << " matches." << std::endl;
	if (bestcnt < 4) {
	    showStatus("Not enough calibration points to compute homography to "+(curUnit<nunits?"laser "+std::to_string(curUnit+1):"world")+"; only have "+std::to_string(bestcnt)+"/4 points.");
	    resultCode = -1;
	    break;
	}
	std::vector<cv::Point2f> src,dst;

	for (int j=0;j<pairwiseMatches.size();j++) {
	    cv::detail::MatchesInfo pm = pairwiseMatches[j];
	    if (found[pm.src_img_idx] && pm.dst_img_idx==curUnit) {
		for (int i=0;i<pm.matches.size();i++) {
		    // Project the point
		    std::vector<cv::Point2f> tmp(1),mapped(1);
		    tmp[0]=features[pm.src_img_idx].keypoints[pm.matches[i].queryIdx].pt;
		    cv::perspectiveTransform(tmp,mapped,homographies[pm.src_img_idx]);
		    src.push_back(mapped[0]);
		    dst.push_back(features[pm.dst_img_idx].keypoints[pm.matches[i].trainIdx].pt);
		    dbg("Calibration.recompute",2) << pm.dst_img_idx << "." << pm.matches[i].trainIdx << std::endl;
		}
	    }
	    else if (found[pm.dst_img_idx] && pm.src_img_idx==curUnit) {
		for (int i=0;i<pm.matches.size();i++) {
		    // Project the point
		    std::vector<cv::Point2f> tmp(1),mapped(1);
		    tmp[0]=features[pm.dst_img_idx].keypoints[pm.matches[i].trainIdx].pt;
		    cv::perspectiveTransform(tmp,mapped,homographies[pm.dst_img_idx]);
		    src.push_back(mapped[0]);
		    dst.push_back(features[pm.src_img_idx].keypoints[pm.matches[i].queryIdx].pt);
		}
	    }
	}
	assert(src.size()==bestcnt);
	dbg("Calibration.recompute",2) << "Computing homography: " << std::endl;
	for (int k=0;k<src.size();k++) {
	    dbg("Calibration.recompute",2) << "   " << src[k] << "    " << dst[k]  << std::endl;
	}
	homographies[curUnit]=cv::findHomography(dst,src);
	flatMat(DbgFile(dbgf__,"Calibration.recompute",1) << "Homography for laser " << curUnit << " = \n",homographies[curUnit]) << std::endl;
	found[curUnit]=true;
    }
    
    // Evaluate matches
    for (int i=0;i<pairwiseMatches.size();i++) {
	cv::detail::MatchesInfo p = pairwiseMatches[i];
	if (p.matches.size()==0)
	    continue; 
	cv::Mat_<float> H1 = homographies[p.src_img_idx];
	cv::Mat_<float> H2 = homographies[p.dst_img_idx];

	std::vector<cv::Point2f> p1, p2;
	for (int j=0;j<p.matches.size();j++) {
	    cv::DMatch m=p.matches[j];
	    p1.push_back(features[p.src_img_idx].keypoints[m.queryIdx].pt);
	    p2.push_back(features[p.dst_img_idx].keypoints[m.trainIdx].pt);
	}
	std::vector<cv::Point2f> flat1,flat2;
	cv::perspectiveTransform(p1,flat1,H1);
	cv::perspectiveTransform(p2,flat2,H2);
	for (int j=0;j<p.matches.size();j++) {
	    Point f1=Point(flat1[j].x,flat1[j].y);
	    Point f2=Point(flat2[j].x,flat2[j].y);
	    Point dev1=Lasers::instance()->getLaser(p.src_img_idx)->getTransform().flatToDevice(f1);
	    Point dev2=Lasers::instance()->getLaser(p.dst_img_idx)->getTransform().flatToDevice(f2);
	    Point ferror=f2-f1;
	    Point error=dev2-dev1;
	    dbg("Calibration.recompute",1) << "L" << p.src_img_idx << "@" << f1 << "; " << dev1 << " - L" << p.dst_img_idx  << "@" << f2 << "; " << dev2 << " e=" << error << ", rms=" << ferror.norm() << "; " << error.norm() << std::endl;
	}
    }
    // Evaluate matches
    for (int i=0;i<relMappings.size();i++) {
	if (relMappings[i]->getUnit(0) < homographies.size() &&relMappings[i]->getUnit(1) < homographies.size()) {
	    cv::Mat_<float> H1 = homographies.at(relMappings[i]->getUnit(0));
	    cv::Mat_<float> H2 = homographies.at(relMappings[i]->getUnit(1));
	    relMappings[i]->updateErrors(H1,H2);
	}
    }
    
    return resultCode;
}

// Map to/from device coordinates (-32767:32767 for lasers, 
Point Calibration::map(Point p, int fromUnit, int toUnit) const {
    if (toUnit<0)
	toUnit=nunits;	// Map to world 
    std::vector<cv::Point2f> p1, p2, world;
    Point flat1;
    if (fromUnit<nunits)
	flat1=Lasers::instance()->getLaser(fromUnit)->getTransform().deviceToFlat(p);
    else
	flat1=p;
	
    p1.push_back(cv::Point2f(flat1.X(),flat1.Y()));

    cv::perspectiveTransform(p1,world,homographies[fromUnit]);
    cv::perspectiveTransform(world,p2,homographies[toUnit].inv());

    Point flat2=Point(p2[0].x,p2[0].y);
    Point dev2;

    if (toUnit<nunits)
	dev2=Lasers::instance()->getLaser(toUnit)->getTransform().flatToDevice(flat2);
    else
	dev2=flat2;
    
    dbg("Calibration.map",2) << "Mapped " << p << " on unit " << fromUnit << " to " << dev2 << " on  unit " << toUnit << std::endl;
    return dev2;
}

std::vector<float> RelMapping::updateErrors() const {
    std::vector<float> error(pt1.size());
    for (int j=0;j<pt1.size();j++) {
	Point w1=Calibration::instance()->map(getDevicePt(0,j),unit1);
	Point w2=Calibration::instance()->map(getDevicePt(1,j),unit2);
	Point err=w2-w1;
	dbg("relMappings.updateErrors",1) << "L" << unit1 << "@" << pt1[j] << " -> " << w1 << " - L" << unit2  << "@" << pt2[j] << " -> " << w2  << ", e=" << err << ", rms=" << err.norm() << std::endl;
	error[j]=err.norm();
    }
    return error;
}

// Get the set of calibration points that should be drawn for the given laser
std::vector<Point> Calibration::getCalPoints(int unit) const {
    std::vector<Point> result;
    switch (laserMode) {
    case CM_NORMAL:
	// Nothing to do
	break;
    case CM_CURPT:
	result = curMap->getCalPoints(unit,true);
	break;
    case CM_CURPAIR:
	result = curMap->getCalPoints(unit,false);
	break;
    case CM_ALL:
	for (int i=0;i<relMappings.size();i++) {
	    std::vector<Point> p=relMappings[i]->getCalPoints(unit,false);
	    result.insert(result.end(),p.begin(),p.end());
	}
	break;
    }
    return result;
}

// Get the set of calibration points that should be drawn for the given laser
std::vector<Point> RelMapping::getCalPoints(int unit, bool selectedOnly) const {
    std::vector<Point> result;
    const std::vector<Point> *p = 0;	// Point to point list for given unit
    if (unit1==unit)
	p=&pt1;
    else if (unit2==unit)
	p=&pt2;
    else
	// No match
	return result;
    if (selectedOnly)
	result.push_back((*p)[selected]);	// Only selected point
    else
	result = *p;		// All points

    return result;
}
