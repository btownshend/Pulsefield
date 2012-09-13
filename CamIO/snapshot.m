% Take a snapshot from the cameras
function snapshot(p)
ids=1:6
if nargin==1
  % Turn off any sensor cropping
  sensorcrop(p,true);
end
im=aremulti(ids,'av10115-half');

if nargin==1
  % Turn sensor corpping back on
  sensorcrop(p);
end
fname=sprintf('/Users/bst/Dropbox/PeopleSensor/Snapshots/%s.mat',datestr(now,30));
fprintf('Saving snapshot in %s\n', fname);
snapshot=struct('when',now,'im',im);
save(fname,'snapshot');
setfig('snapshot');
for i=1:length(ids)
  subplot(2,ceil(length(ids)/2),i)
  imshow(im{i});
end
