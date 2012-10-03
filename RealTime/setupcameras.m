% Set all cameras to controlled settings
function setupcameras(p,varargin)
defaults=struct('mode','highspeed','exptime',10,'daynight','day');
args=processargs(defaults,varargin);

if ~any(strcmp(args.mode,{'highspeed','quality','moonlight'}))
  error('setupcameras: bad mode: %s\n', args.mode);
end

if ~any(strcmp(args.daynight,{'day','night'}))
  error('setupcameras: daynight mode should be "day" or "night"\n');
end

if strcmp(args.mode,'highspeed')
  args.exptime=round(args.exptime);
  if args.exptime<1 || args.exptime>80
    error('setupcameras: exposure time must be between 1 and 80\n');
  end
end
fprintf('Setting cameras to mode %s, exposure time %d\n',args.mode,args.exptime);
for id=[p.camera.id]
  arecont_set(id,'1080p_mode','off');
  arecont_set(id,'day_binning','off');
  arecont_set(id,'night_binning','off');
  arecont_set(id,'autoexp','on');
  arecont_set(id,'exposure','on');
  arecont_set(id,'brightness',0);
  arecont_set(id,'lowlight',args.mode);
  arecont_set(id,'shortexposures',args.exptime);
  arecont_set(id,'maxdigitalgain',32);
  arecont_set(id,'analoggain',1);
  arecont_set(id,'illum','outdoor');
  if id~=1
    arecont_set(id,'daynight',args.daynight);
  end
end
