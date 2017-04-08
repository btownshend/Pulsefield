#include <stdio.h>
#include "normal.h"

int main() {
    float nu=384.827;
    float sigma=15.794;
    double value=795.568;
    printf("ricepdf(%f,%f,%f)=%g\n", value,nu,sigma,ricepdf(value,nu,sigma));
    for(int i=0;i<20;i++) {
	value=nu+i*sigma;
	double arg=value*nu/(sigma*sigma);
	printf("bessi0(%f)=%g\n", arg, bessi0(arg));
	printf("i=%d, ricepdf(%f,%f,%f)=%g\n", i, value,nu,sigma,ricepdf(value,nu,sigma));
    }
    sigma=105;
    float dclr=1.272;
    float diam=200;
    printf("ricecdf(%f,%f,%f)=%g\n", dclr,diam/2,sigma,ricecdf(dclr,diam/2,sigma));
    float step=0.01;
    float sum=0;
    for (int i=0;i<5/step;i++) {
	float x=i*step;
	float rpdf=ricepdf(x,1,1);
	float rcdf=ricecdf(x,1,1);
	sum+=rpdf*step;
	if (i%10==0)
	    printf("x=%.1f, pdf=%.2f, cdf=%.2f, sum=%.2f\n",x,rpdf,rcdf,sum);
    }
}
