% Diagnostic plots/output
function diagnostic(snap)
bg=snap(end).bg;
vis=snap(end).vis;
tracker=snap(end).tracker;
MAXSPECIAL=2;
fprintf('\n');
setfig('diagnostic');clf;
hold on;

xy=range2xy(vis.angle+pi/2,vis.range);
bxy=range2xy(bg.angle+pi/2,bg.range);

col='gbcymk';
plotted=false(size(vis.class));
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  k=t.kalmanFilter;
  loc=t.updatedLoc;
  vel=k.State([2,4])';
  if ~isempty(t.predictedLoc) && ~isempty(t.measuredLoc)
    error=norm(t.predictedLoc-t.measuredLoc);
  else
    error=nan;
  end
  fprintf('Track %d: MSE=%.3f age=%d, visCount=%d, consInvis=%d, loc=(%.1f,%1.f), velocity=(%.1f,%.1f), bbox=(%.1f,%.1f,%.1f,%.1f)\n', t.id, sqrt(mean(error.^2)), t.age, t.totalVisibleCount, t.consecutiveInvisibleCount, t.updatedLoc, vel, t.bbox);
  color=col(min(i,length(col)));
  if ~isempty(t.predictedLoc)
    plot(t.predictedLoc(1),t.predictedLoc(2),['o',color]);
  end
  if ~isempty(t.measuredLoc)
    plot(t.measuredLoc(1),t.measuredLoc(2),['x',color]);
  end
  plot(t.updatedLoc(1),t.updatedLoc(2),['+',color]);
  asel=tracker.assignments(:,1)==i;
  if sum(asel)==1
    det=tracker.assignments(asel,2);
    cnum=det+2;
    fprintf('det=%d, class=%d, npts=%d\n',det,cnum,sum(vis.class==cnum));
    plot(vis.targets.pos(det,1),vis.targets.pos(det,2),['x',color]);
    sel=vis.class==cnum;
    lsel=sel& (vis.leg==2);
    rsel=sel& (vis.leg==1);
    osel=sel&~lsel&~rsel;
    plot(xy(osel,1),xy(osel,2),['.',color]);
    plot(xy(lsel,1),xy(lsel,2),['<',color]);
    plot(xy(rsel,1),xy(rsel,2),['>',color]);
    plotted=plotted|sel;
  end
end
if sum(~plotted)>0
  sel=~plotted & vis.class>MAXSPECIAL;
  if sum(sel)>0
    fprintf('%d target points not matched to tracks\n', sum(sel));
  end
  plot(xy(sel,1),xy(sel,2),'.r');
  sel=~plotted & vis.class>0&vis.class<=MAXSPECIAL;
  plot(xy(sel,1),xy(sel,2),'.k');
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

