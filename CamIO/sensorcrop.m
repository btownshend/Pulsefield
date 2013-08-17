% Set sensor cropping for active cameras
% Note that the cameras reset this when a new stream is begun
function sensorcrop(p,off)
if nargin<2
  off=false;
end
if off
  fprintf('Setting sensor cropping to off\n');
else
  fprintf('Setting sensor cropping to on\n');
end
for i=1:length(p.camera)
  c=p.camera(i);
  roi=c.roi;
  if strcmp(c.type,'av10115-half')
    % Need full size roi
    roi=(roi-1)*2+1;
  end
  if off
    roi=[0 2751 0 2751];
  end
  fprintf('Setting camera ID %d to sensortop=%d, sensorheight=%d...',c.id,roi(3),roi(4)-roi(3)+1);
  for retry=1:5
    arecont_set(c.id,'sensortop',roi(3));
    arecont_set(c.id,'sensorheight',roi(4)-roi(3)+1);
    pause(0.1);
    check=arecont_get(c.id,'sensorheight');
    if abs(check- (roi(4)-roi(3)+1)) > 10
      fprintf('\nWARNING: failed to set camera %d sensor height to %d; reads back as %d\n', c.id, roi(4)-roi(3)+1, check);
      pause(2);
    else
      fprintf('done\n');
      break
    end
    if retry==5 && check>500
      error('Unable to set camera %d sensor height -- is a web window open?\n',c.id);
    end
  end
end
  