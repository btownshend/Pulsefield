#ifndef NORMAL_H_
#define NORMAL_H_

#include <math.h>

inline double normcdf(double value,double mu=0.0, double sigma=1.0) {
    double z=(value-mu)/sigma;
    return 0.5 * erfc(-z * M_SQRT1_2);
}

inline double normpdf(double value, double mu=0.0, double sigma=1.0) {
    // NOTE: probability densities are prob/unit of the input, so if its in mm, the result would be 1000 times more than if it is meters.
    static const double inv_sqrt_2pi = 0.3989422804014327; 
    double z=(value-mu)/sigma;
    return inv_sqrt_2pi / sigma*exp(-0.5 * z * z);
}
#endif /* NORMAL_H_ */
