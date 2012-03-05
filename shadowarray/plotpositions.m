% plotpositions - plot positions of cameras, leds
function plotpositions(cp,cd,lp,p)
clf;
hold on
rng=max(cp(:,1))-min(cp(:,1));
for c=1:size(cp,1)
  plot(cp(c,1),cp(c,2),'go');
  quiver(cp(c,1),cp(c,2),cd(c,1)*rng/20,cd(c,2)*rng/20,'g');
end
for l=1:size(lp,1)
  plot(lp(l,1),lp(l,2),'r.','MarkerSize',1);
end
axis equal
axis ij

