% Test PDF of lognorm vs norm

mu=300;
sigma=100;
d=1:mu+3*sigma;
scale=1000;

llin=normpdf(d,mu,sigma)*scale;

llog=normpdf(log(d),log(mu),log(1+sigma/mu)).*(scale/mu);

clf;
plot(d,llin);
hold on;
plot(d,llog,'r');
