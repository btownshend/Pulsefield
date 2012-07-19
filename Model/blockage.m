% Check blockage as a function of number of people
ncamera=8;	% Number of cameras
nsegs=8;	% Number of segments in pulsefield
tgtdiam=0.4;    % Target diameter
pctfunc=.9;	% Frac of Camera->LED lines that are functional
maxpeople=20;
mincameras=3;

seglen=8*12/39.37;
pfcirc=(nsegs+1/4)*seglen;
pfdiam=pfcirc/pi;
pfarea=pi*(pfdiam/2)^2;

pblock=1.75*tgtdiam*pfdiam/pfarea   % Probability that a person is shadowed

P=[];func=[];
for n=1:maxpeople  % Number of people in pulsefield
  pcamblocked=1-pctfunc*(1-pblock)^(n-1);
  for k=0:ncamera	% Number of cameras visible
    P(n,k+1)=nchoosek(ncamera,k)*(1-pcamblocked)^k * pcamblocked^(ncamera-k);
  end
end

setfig('blockage');
clf;hold on;
col='rgbcmykrgbcmyk';
h=[];leg={};
for i=1:ncamera
  % i is num cameras visible
  func=sum(P(:,(i+1):end),2);  
  h(i)=plot(func,col(i));
  leg{i}=sprintf('>=%d visible',i);
end
legend(h,leg);
axis([1,maxpeople,0,1]);
xlabel('Number of people');
ylabel('Fraction of samps operational');
title(sprintf('Fraction of samples functional using %d cameras, %d segments',ncamera, nsegs));
