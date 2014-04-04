% Diagnostic plots/output
function plotsnap(snap,varargin)
defaults=struct('frame',[],...
                'debug',false...
                );
args=processargs(defaults,varargin);

frames=arrayfun(@(z) z.vis.frame, snap);

if ~isempty(args.frame)
  index=find(args.frame==frames);
  if isempty(index)
    fprintf('Frame not found\n');
    return;
  end
  fprintf('Showing snap(%d)\n', index);
  snap=snap(index);
elseif length(snap)>1
  fprintf('plotsnap takes a single snap entry as arg, or a frame optional arg\n')
end

bg=snap.bg;
vis=snap.vis;
tracker=snap.tracker;
MAXSPECIAL=2;
fprintf('\n');
setfig(sprintf('Frame %d',snap.vis.frame));clf;
hold on;

xy=range2xy(vis.angle,vis.range);

colors='rgbcmk';
plotted=false(size(vis.class));
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  id=t.id;
  loc=t.position;
  vel=t.velocity;
  fprintf('%s\n', t.tostring());
  color=colors(mod(id-1,length(colors))+1);
  % plot(t.updatedLoc(1),t.updatedLoc(2),['+',color]);
  plot(t.position(1),t.position(2),['x',color]);
  plot(t.legs(:,1),t.legs(:,2),['o',color]);
  %  cnum=t.legclasses;
  %  cnum(cnum==1)=10000;   % Different from any class
  %fprintf('class=(%d,%d), npts=(%d,%d)\n',cnum,sum(vis.class==cnum(1)),sum(vis.class==cnum(2)));
  %  lsel=vis.class==cnum(1);
  %  rsel=vis.class==cnum(2);
  lsel=tracker.assignments(:,1)==i & tracker.assignments(:,2)==1;
  rsel=tracker.assignments(:,1)==i & tracker.assignments(:,2)==2;
  plot(xy(lsel,1),xy(lsel,2),['<',color]);
  plot(xy(rsel,1),xy(rsel,2),['>',color]);
  plotted=plotted|lsel|rsel;
  for l=1:2
    % Draw legs
    leg=t.legs(l,:);
    angle=-pi:pi/20:pi;
    [x,y]=pol2cart(angle,t.legdiam/2);
    x=x+leg(1);
    y=y+leg(2);
    plot(x,y,color);
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

plot(xy(:,1),xy(:,2),'g');

bfreq=bg.freq/max(sum(bg.freq(1:2,:),1));
bfreq(3,:)=1-sum(bfreq(1:2,:),1);
[~,maxb]=max(bfreq,[],1);
range=[];
for i=1:length(maxb)
  range(i)=bg.range(maxb(i),i);
end
params=getparams();
range(maxb==3)=params.maxrange;
bxy=range2xy(bg.angle,range);
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

end
