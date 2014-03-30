% Recompute given frame from snap
function recompute(snap,frame)
params=getparams();

frames=arrayfun(@(z) z.vis.frame, snap);
index=find(frame==frames);
if isempty(index)
  fprintf('Frame not found\n');
  return;
end
vis=snap(index).vis;
if index==1
  npredict=1
  bg=Background(vis);
else
  npredict=frame-snap(index-1).vis.frame;
  bg=snap(index-1).bg.clone();
  bg.update(vis);
end
fprintf('Recomputing snap(%d) - frame %d with %d frame steps\n', index,frame,npredict);
vis=classify(vis,bg,'debug',true);
vis=splitclasses(vis,'debug',true);
first=find(vis.class>1,1);
last=find(vis.class>1,1,'last');
fprintf('classes(%d:%d)=%s\n', first,last,sprintf('%d ',vis.class(first:last)));
if index==1
  tracker=World();
else
  tracker=snap(index-1).tracker.clone();
end
tracker.debug=true;
for i=1:length(tracker.tracks)
  tracker.tracks(i).debug=true;
end
tracker.update(vis,npredict,params.fps);
plotsnap(struct('vis',vis,'tracker',tracker,'bg',snap(index).bg));
