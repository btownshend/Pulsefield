% Calculate background noise model using vis data
function nm=noisemodel(p,vis,cam)
nm.avg=vis.refim{cam};
nm.var=max(0,vis.refim2{cam}-vis.refim{cam}.^2);
% Make sure the std isn't too small
cliplev=p.analysisparams.fgminvar;
fprintf('Clipping at %.3f\n', cliplev);
nm.var(nm.var>=0 & nm.var<cliplev)=cliplev;
nm.var(nm.var<0 & nm.var>-cliplev)=-cliplev;

