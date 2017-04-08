function plotlike(snap,frame)
frames=arrayfun(@(z) z.vis.frame,snap);
index=find(frames==frame);
if isempty(index)
  error('Frame %d not found in snap: range is %d-%d\n', frame, min(frames), max(frames));
end
if length(snap(index).tracker.tracks)==0
  error('Frame %d ( snap(%d) )does not contain any tracks\n', frame,index);
end
for i=1:length(snap(index).tracker.tracks)
  snap(index).tracker.tracks(i).plotlike(snap(index).vis);
end