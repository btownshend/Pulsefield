function p=arecont_getall(id)
% Load image and parameters from arecont
params={'kneepoint','analoggain','maxkneegain','maxexptime','maxdigitalgain','brightness','sharpness','lowlight','shortexposures','autoexp','exposure','imgquality','imgres','rotate','saturation','blue','red','illum','freq','expwndleft','expwndtop','expwndwidth','expwndheight','sensorleft','sensortop','sensorwidth','sensorheight','maxsensorwidth','maxsensorheight','imgleft','imgtop','imgwidth','imgheight','mac','model','fwversion','procversion','netversion','auxin','auxout','cropping','day_binning','night_binning','casino_mode','fps','reg_3_209','reg_3_210'};
broken_params={'channelenable','1080p_mode'};
p=struct();
for i=1:length(params)
  p.(params{i})=arecont_get(id,params{i});
end
