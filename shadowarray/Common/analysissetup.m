function aparams=analysissetup
waistcircum=1;
wcsigma=0.14;
meantgtdiam=waistcircum/pi;
sdevtgtdiam=wcsigma/pi;
aparams.mintgtdiam=meantgtdiam/1.2-sqrt(3)*sdevtgtdiam;  % 1.2 for eccentricity of shapes
aparams.maxtgtdiam=meantgtdiam*1.2+sqrt(3)*sdevtgtdiam;
aparams.npixels=1000;