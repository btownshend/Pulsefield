% Diagnostic plots/output
function diagnostic(bg,vis,nexttracker,tracker)
MAXSPECIAL=2;
fprintf('\n');
setfig('diagnostic');clf;

xy=range2xy(vis.angle+pi/2,vis.range);
bxy=range2xy(bg.angle+pi/2,bg.range);
sel=vis.class>MAXSPECIAL;
plot(xy(sel,1),xy(sel,2),'.r');
hold on;
sel=vis.class>0&vis.class<=MAXSPECIAL;
plot(xy(sel,1),xy(sel,2),'.k');

col='gbcymk';
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  k=t.kalmanFilter;
  loc=k.State([1,3])';
  vel=k.State([2,4])';
  fprintf('Track %d: MSE=%.3f age=%d, visCount=%d, consInvis=%d, loc=(%.1f,%1.f), velocity=(%.1f,%.1f), bbox=(%.1f,%.1f,%.1f,%.1f)\n', t.id, sqrt(mean(t.error.^2)), t.age, t.totalVisibleCount, t.consecutiveInvisibleCount, loc, vel, t.bbox);
  color=col(min(i,length(col)));
  plot(loc(:,1),loc(:,2),['o',color]);
  nexttracks=nexttracker.tracks;
  nextsel=[nexttracks.id]==t.id;
  if sum(nextsel)==1
    % Show prior position
    prevloc=nexttracks(nextsel).kalmanFilter.State([1,3])';
    plot([prevloc(:,1),loc(:,1)],[prevloc(:,2),loc(:,2)],color);
  else
    fprintf('No next track\n');
  end
  asel=tracker.assignments(:,1)==i;
  if sum(asel)==1
    det=tracker.assignments(asel,2);
    cnum=det+2;
    fprintf('det=%d, class=%d, npts=%d\n',det,cnum,sum(vis.class==cnum));
    plot(vis.targets.pos(det,1),vis.targets.pos(det,2),['x',color]);
    sel=vis.class==cnum;
    plot(xy(sel,1),xy(sel,2),['.',color]);
  end
end
plot(bxy(:,1),bxy(:,2),'k');
axis equal;
xyt=xy(vis.class>MAXSPECIAL,:);
if ~isempty(xyt)
  c=[min(xyt(:,1)),max(xyt(:,1)),min(xyt(:,2)),max(xyt(:,2))];
  ctr=(floor(c([1,3]))+ceil(c([2,4])))/2;
  sz=max(ceil(c([2,4]))-floor(c([1,3])));
  newc=[ctr(1)-sz/2,ctr(1)+sz/2,ctr(2)-sz/2,ctr(2)+sz/2];
  axis(newc);  % Zoom to ROI
end

