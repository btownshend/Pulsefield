// Test Gaussian mixtures
#include "gaussianmixture.h"

int main(int argc, char *argv[]) {
    const float vdata[]={0.4,0.5,0.6,10.0,10.1,10.2,10.3,10.4,10.5};
    std::vector<float> data(vdata,vdata+sizeof(vdata)/sizeof(vdata[0]));
    float testval=0.5;
    const int nclusters=2;
    GaussianMixture g(nclusters,100);
    std::cout << "G=" << g << std::endl;
    for (int i=0;i<data.size();i++) {
	g.add(data[i]);
	std::cout << "add " << data[i] << std::endl;
	if (i>=nclusters) {
	    g.retrain();
	    std::cout << "G=" << g << std::endl;
	    std::cout << "g.like(" << testval << ")=" << g.like(testval) << std::endl;
	}
    }
    for (int i=0;i<sizeof(data)/sizeof(data[0]);i++) {
	float x=data[i];
	std::cout << "g.like(" << x << ")=" << g.like(x) << std::endl;
    }
}
