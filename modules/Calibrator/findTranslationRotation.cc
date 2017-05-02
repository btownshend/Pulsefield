#include "dbg.h"
#include "findTranslationRotation.h"

// Find a 3x3 matrix that maps src points to dst points using only rotation and translation
cv::Mat findTranslationRotation( const std::vector<cv::Point2f> &src,  const std::vector<cv::Point2f> &dst) {
    int npoints=src.size();
    dbg("findTranslationRotation",1)  << "findTranslationRotation(" << src << "," << dst << ")" << ", npoints=" << npoints << std::endl;
    cv::Point2f srcCentroid,dstCentroid;
    for (int i=0;i<npoints;i++) {
	srcCentroid+=src[i];
	dstCentroid+=dst[i];
    }
    srcCentroid/=npoints;
    dstCentroid/=npoints;
    dbg("findTranslationRotation",2)  << "src centroid=" << srcCentroid << ", dstCentroid=" << dstCentroid << std::endl;

    cv::Mat H=cv::Mat::zeros(2,2,CV_32F);
    for (int i=0;i<npoints;i++) {
	H+=cv::Mat(src[i]-srcCentroid)*cv::Mat(dst[i]-dstCentroid).t();
    }
    dbg("findTranslationRotation",2)  << "H=" << H << std::endl;
    cv::Mat w,u,vt;
    cv::SVD::compute(H, w, u, vt);
    cv::Mat v=vt.t();
    dbg("findTranslationRotation",2)  << "U=" << u << std::endl;
    dbg("findTranslationRotation",2)  << "V=" << v << std::endl;
    cv::Mat r=v*u.t();
    dbg("findTranslationRotation",2)  << "r=" << r << std::endl;
    float det=determinant(r);
    dbg("findTranslationRotation",2) << "det=" << det << std::endl;
    if (det<0) {
	v.at<double>(0,1)=-v.at<double>(0,1);
	v.at<double>(1,1)=-v.at<double>(1,1);
	r=v*u.t();
	det=determinant(r);
	//	assert(det>0);
    }
    // Compute translation
    cv::Mat t=-r*cv::Mat(srcCentroid)+cv::Mat(dstCentroid);
    dbg("findTranslationRotation",2)  << "t=" << t << std::endl;
    // Combine into a homogenous matrix
    cv::Mat result=cv::Mat::zeros(3, 3, CV_64F);	// Make it a default transform
    result.at<double>(0,2)=t.at<float>(0,0);
    result.at<double>(1,2)=t.at<float>(1,0);
    for (int i=0;i<2;i++)
	for (int j=0;j<2;j++)
	    result.at<double>(i,j)=r.at<float>(i,j);
    result.at<double>(2,2)=1;
    dbg("findTranslationRotation",1)  << "-> " << result << std::endl;
    return result;
}
