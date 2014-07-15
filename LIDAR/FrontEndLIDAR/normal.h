#pragma once

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


// Rayleigh distribution - distribution of magnitude of a 2d multivariate gaussian with uncorrelated components, equal variance, zero mean
inline double rayleighpdf(double value, double sigma=1.0) {
    double z=value/sigma;
    return value/(sigma*sigma)*exp(-0.5*z*z);
}

inline double rayleighcdf(double value, double sigma=1.0) {
    double z=value/sigma;
    return 1- exp(-0.5*z*z);
}

inline double bessi0( double x )
/*------------------------------------------------------------*/
/* PURPOSE: Evaluate modified Bessel function In(x) and n=0.  */
/*------------------------------------------------------------*/
{
   double ax,ans;
   double y;


   if ((ax=fabs(x)) < 3.75) {
      y=x/3.75,y=y*y;
      ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
         +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
   } else {
      y=3.75/ax;
      ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
         +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
         +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
         +y*0.392377e-2))))))));
   }
   return ans;
}

// Rice distribution - distribution of distance between two 2d multivariate gaussian with uncorrelated components, equal variance
inline double ricepdf(double value, double nu, double sigma) {
    double arg=value*nu/(sigma*sigma);
    double result= value/(sigma*sigma)*exp(-(value*value+nu*nu)/(2*sigma*sigma))*bessi0(arg);
    if (isnan(result) || isinf(result))
	result=0;
    return result;
}

inline double ricecdf(double value, double nu, double sigma) {
    if (value==0)
	return 0.0;
    float sum=0;
    float step=sigma/10;
    float x0=fmax(0,nu-sigma*3);
    float x1=fmin(value,nu+sigma*3);
    if (x0>=x1)
	x0=0;
    int nstep=(int)((x1-x0)/step);
    if (nstep<5)
	nstep=5;
    step=(x1-x0)/(nstep-1);
    //printf("x0=%f, x1=%f, step=%f, nstep=%d\n", x0, x1, step, nstep);
    for (int i=0;i<nstep;i++) {
	float x=x0+i*step;
	sum+=ricepdf(x,nu,sigma);
    }
    return sum*step;
}
