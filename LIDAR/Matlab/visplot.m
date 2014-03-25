function visplot(vis)
xy=range2xy(vis.angle,vis.range);
setfig('visplot');clf;
hold on;
if isfield(vis,'targets')
  col=vis.class+1;
else
  col=1;
end
colors='rgbcmk';
col(col>length(colors))=length(colors);
for i=1:size(xy,1)
  plot(xy(i,1),xy(i,2),['.',colors(col(i))]);
end
hold on;
if isfield(vis,'targets') && size(vis.targets.pos,1)>0
  plot(vis.targets.pos(:,1),vis.targets.pos(:,2),'o');
end
if isfield(vis,'truth')
  t=vis.truth;
  plot(0,0,'o');
  if size(t.targets,1)>0
    plot(t.targets(:,1),t.targets(:,2),'x');
  end
end
axis equal;
