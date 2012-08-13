function aparams=analysissetup
waistcircum=0.96;   % From http://www.cdc.gov/nchs/data/nhsr/nhsr010.pdf
waistcircum5pct=0.75;
waistcircum95pct=1.26;
ellipseratio=1.2;
eccentricity=sqrt(1-(1/ellipseratio)^2);
[~,Ee]=ellipke(eccentricity);
aparams.mintgtdiam=waistcircum5pct/(4*Ee);
aparams.maxtgtdiam=waistcircum95pct/(4*Ee)*ellipseratio;

% Maximum false gap (in m), such as rays that go between the legs of somebody
aparams.maxfalsegap=0.1;   % 10cm

%wcsigma=0.14;
%meantgtdiam=waistcircum/pi;
%sdevtgtdiam=wcsigma/pi;
%aparams.mintgtdiam=meantgtdiam/1.2-sqrt(3)*sdevtgtdiam;  % 1.2 for eccentricity of shapes
%aparams.maxtgtdiam=meantgtdiam*1.2+sqrt(3)*sdevtgtdiam;
aparams.npixels=500;

aparams.fps=0;    % Max speed for AV10115 (unless sensorheight settings changed)
aparams.updatetc=60;  % Time constant for update of reference image
aparams.mincorr=0.5;