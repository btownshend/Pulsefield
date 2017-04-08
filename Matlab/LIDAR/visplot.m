function visplot(vis)
xy=range2xy(vis.angle,vis.range);
setfig('visplot');clf;
hold on;
col=ones(size(xy,1),1);
colors='rgbcmk';
col(col>length(colors))=length(colors);
for i=1:size(xy,1)
  plot(xy(i,1),xy(i,2),['.',colors(col(i))]);
end
hold on;
if isfield(vis,'truth')
  t=vis.truth;
  plot(0,0,'o');
  if size(t.targets,1)>0
    plot(t.targets(:,1),t.targets(:,2),'x');
  end
end
axis equal;
