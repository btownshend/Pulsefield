% Take a snapshot from the cameras
function snapshot(p,doplot)
if nargin<2
  doplot=false;
end
ids=1:6
if nargin>=1
  % Turn off any sensor cropping
  sensorcrop(p,true);
  pause(0.1);
end
im=aremulti(ids,'av10115-half');

if nargin>=1
  % Turn sensor cropping back on
  sensorcrop(p);
end
fname=sprintf('/Users/bst/Dropbox/PeopleSensor/Snapshots/%s.mat',datestr(now,30));
fprintf('Saving snapshot in %s\n', fname);
snapshot=struct('when',now,'im',im);
save(fname,'snapshot');
if doplot
  snapshow(snapshot);
  pause(0.1);
end
