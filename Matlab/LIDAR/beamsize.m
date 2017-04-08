% Compute beam size as a function of distance
divergence=1;   % mrad = mm/meter
exitdiam=3; 	% mm
h=4*12/39.37; 	% height off ground
d=0:0.1:10		% Distance
l=sqrt(d.^2+h^2);
beam=exitdiam+divergence*l;
angle=atan(h./d);
sz=beam./sin(angle);
setfig('beamsize');
plot(d,sz);
xlabel('Distance (m)');
ylabel('Line width (mm)');
title(sprintf('Line width for h=%.1fm, div=%.1fmrad, exitdiam=%.1fmm', h, divergence, exitdiam));
