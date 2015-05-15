function [fit,err]=orthfit(x,y)
n=length(x);
xbar=mean(x);
ybar=mean(y);
sxx=1/(n-1)*sum((x-xbar).^2);
sxy=1/(n-1)*sum((x-xbar).*(y-ybar));
syy=1/(n-1)*sum((y-ybar).^2);
b1=(syy-sxx+sqrt((syy-sxx)^2+4*sxy^2))/(2*sxy);
b0=ybar-b1*xbar;
fit=[b1,b0];
py=b1*x+b0;
err=sum(sum((py-y).^2,2));

