% Plot overlaid map of all frames of snap
function plotmap(snap)
setfig('plotmap');clf;
fuzz=.01;
da=snap(1).vis.angle(2)-snap(1).vis.angle(1);
divergence=.011;  % 11 mrad
for i=1:length(snap)
  v=snap(i).vis;
  xy=range2xy(v.angle+(rand(size(v.angle))-0.5)*divergence,v.range);
  plot(xy(:,1),xy(:,2),'.','MarkerSize',2);
  hold on;
end
setfig('plotmap2');clf;
for i=1:length(snap)
  v=snap(i).vis;
  plot(v.angle+(rand(size(v.angle))-0.5)*da,v.range(:),'.','MarkerSize',2);
  hold on;
end

       
