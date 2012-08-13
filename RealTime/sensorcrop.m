% Set sensor cropping for active cameras
% Note that the cameras reset this when a new stream is begun
function sensorcrop(p,off)
if nargin<2
  off=false;
end
for i=1:length(p.camera)
  c=p.camera(i);
  roi=c.roi;
  if strcmp(c.type,'av10115-half')
    % Need full size roi
    roi=(roi-1)*2+1;
  end
  if off
    roi=[0 9999 0 9999];
  end
  arecont_set(c.id,'sensortop',roi(3));
  arecont_set(c.id,'sensorheight',roi(4)-roi(3)+1);
  fprintf('Set ID %d to sensortop=%d, sensorheight=%d\n',c.id,roi(3),roi(4)-roi(3)+1);
end
  