% Calculate background noise model using vis data
function nm=noisemodel(vis,cam)
nm.avg=vis.refim{cam};
nm.std=sqrt(vis.refim2{cam}-vis.refim{cam}.^2);
% Make sure the std isn't too small -- use 1/3 of mean as clip
cliplev=mean(nm.std(:))/3;
fprintf('Clipping at %.3f\n', cliplev);
nm.std(nm.std>=0 & nm.std<cliplev)=cliplev;
nm.std(nm.std<0 & nm.std>-cliplev)=-cliplev;

