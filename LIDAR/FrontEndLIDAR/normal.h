#ifndef NORMAL_H_
#define NORMAL_H_

#include <math.h>

inline double normcdf(double value,double mu=0.0, double sigma=1.0) {
    double z=(value-mu)/sigma;
    return 0.5 * erfc(-z * M_SQRT1_2);
}

inline double normpdf(double value, double mu=0.0, double sigma=1.0) {
    // NOTE: Scaling density by 1000 assuming that the incoming values are in mm, but we're working in prob density per meter
    static const double inv_sqrt_2pi = 0.3989422804014327; 
    double z=(value-mu)/sigma;
    return inv_sqrt_2pi / sigma*exp(-0.5 * z * z);
}
#endif /* NORMAL_H_ */
