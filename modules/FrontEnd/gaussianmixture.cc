#include "gaussianmixture.h"
#include "dbg.h"
#include "point.h"
#include "parameters.h"

void GaussianMixture::retrain() {
    if (history.size() < numMixtures) {
	dbg("GaussianMixture.retrain",2) << "Not enough samples to train; have " << history.size() << ", need at least " << numMixtures << std::endl;
	return;
    }
    cv::Mat samples = cv::Mat::zeros(history.size(),1,CV_32FC1);
    int j=0;
    for (std::list<float>::iterator i=history.begin();i!=history.end();i++)
	samples.at<float>(j++,0)=*i;
    if (!model.train(samples))
	dbg("GaussianMixture.retrain",1) << "Failed train with sample=" << samples << std::endl;
    dbg("GaussianMixture.retrain",4) << *this << std::endl;
}

// Get value of PDF at x
float GaussianMixture::like(float x) const {
    if (!model.isTrained())
	return -log(MAXRANGE/10);   // Assume every point within range is equally likely a background pixel
    std::vector<float> sample;
    sample.push_back(x);
    cv::Vec2d result = model.predict(sample);
    return result[0];
}

std::vector<float> GaussianMixture::getMeans() const {
    std::vector<float> means(numMixtures);
    if (model.isTrained()) {
	cv::Mat meanMat=model.get<cv::Mat>("means");
	for (int i=0;i<numMixtures;i++)
	    means[i]=meanMat.at<float>(i);
    }
    return means;
}

std::vector<float> GaussianMixture::getWeights() const {
    std::vector<float> weights(numMixtures);
    if (model.isTrained()) {
	cv::Mat weightMat=model.get<cv::Mat>("weights");
	for (int i=0;i<numMixtures;i++)
	    weights[i]=weightMat.at<float>(i);
    }
    return weights;
}

std::ostream &operator<<(std::ostream &s, const GaussianMixture &g) {
    std::vector<cv::Mat> covs=g.model.get<std::vector<cv::Mat> >("covs") ;
    s << "History: " << g.history.size() << ", weights: "  << g.model.get<cv::Mat>("weights") << ", means=" << g.model.get<cv::Mat>("means") << ", vars:  ";
    for (int i=0;i<covs.size();i++) {
	if (i>0) s<< ",";
	s << covs[i];
    }
    return s;
}
