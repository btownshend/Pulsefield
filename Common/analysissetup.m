function aparams=analysissetup
waistcircum=0.96;   % From http://www.cdc.gov/nchs/data/nhsr/nhsr010.pdf
waistcircum5pct=0.75;
waistcircum95pct=1.26;
ellipseratio=1.2;
eccentricity=sqrt(1-(1/ellipseratio)^2);
[~,Ee]=ellipke(eccentricity);
aparams.mintgtdiam=waistcircum5pct/(4*Ee);
aparams.maxtgtdiam=waistcircum95pct/(4*Ee)*ellipseratio;
%wcsigma=0.14;
%meantgtdiam=waistcircum/pi;
%sdevtgtdiam=wcsigma/pi;
%aparams.mintgtdiam=meantgtdiam/1.2-sqrt(3)*sdevtgtdiam;  % 1.2 for eccentricity of shapes
%aparams.maxtgtdiam=meantgtdiam*1.2+sqrt(3)*sdevtgtdiam;
aparams.npixels=500;
aparams.minmargin=50;  % Minimum margin between on-level and off-level to use a given LED->camera

% Setup of algorithm for getvisible()
aparams.visalgorithm='xcorr';   % 'xcorr' or 'maxlev'

% Paramers for 'maxlev' visalgorithm
aparams.thresh=250;    % LED detection threshold if crosstalk has not been run

% Parameters for correlation-based getvisibible() algorithm (xcorr)
aparams.bbwind=5;   % Number of pixels on either side of centroid to examine (ends up 2*bbwind+1 pixels)
aparams.mincorr=0.5;   % Minimum correlation to assume LED is visible
