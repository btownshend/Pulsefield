% Set all cameras to controlled settings
function setupcameras(p)
fprintf('Setting camera exposures\n');
for id=[p.camera.id]
  arecont_set(id,'1080p_mode','off');
  arecont_set(id,'day_binning','off');
  arecont_set(id,'night_binning','off');
  arecont_set(id,'autoexp','on');
  arecont_set(id,'exposure','on');
  arecont_set(id,'brightness',0);
  arecont_set(id,'lowlight','highspeed');
  arecont_set(id,'shortexposures',2);
  arecont_set(id,'maxdigitalgain',32);
  arecont_set(id,'analoggain',1);
  arecont_set(id,'illum','outdoor');
  if id~=1
    arecont_set(id,'daynight','day');
  end
end
