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
  tracker=World();
  bg=Background(vis);
else
  npredict=frame-snap(index-1).vis.frame;
  tracker=snap(index-1).tracker.clone();
  tracker.debug=true;
  for i=1:length(tracker.tracks)
    tracker.tracks(i).debug=true;
  end
  bg=snap(index-1).bg.clone();
  bg.update(vis);
end
fprintf('Recomputing snap(%d) - frame %d with %d frame steps\n', index,frame,npredict);
vis=classify(vis,bg,'debug',true);
vis=splitclasses(vis,'debug',true);
tracker.update(vis,npredict,params.fps);
