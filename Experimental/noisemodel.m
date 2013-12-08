% Calculate background noise model using vis data
function nm=noisemodel(p,vis,cam)
nm.avg=vis.refim{cam};
nm.var=vis.refim2{cam};
% Make sure the std isn't too small
cliplev=p.analysisparams.fgminvar;
fprintf('Using minimum value of (%.1f/255)^2 for variances\n', 255*sqrt(cliplev));
nm.var(nm.var>=0 & nm.var<cliplev)=cliplev;
nm.var(nm.var<0 & nm.var>-cliplev)=-cliplev;

