% Diagnostic plots/output
function plotsnap(snap,varargin)
params=getparams();
defaults=struct('frame',[],...
                'setfig',true,...
                'maxrange',params.maxrange,...
                'showhits',true,...
                'crop',true,...
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
  if length(index)>1
    fprintf('Have %d copies of frame %d - showing first one\n', length(index), args.frame);
    index=index(1);
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
if args.setfig
  setfig(sprintf('Frame %d',snap.vis.frame));
end
clf;
hold on;

if isfield(vis,'world')
  xy=vis.world;
else
  xy=range2xy(vis.angle,vis.range);
end

colors=get(gca,'ColorOrder');
isbg=tracker.assignments(:,1)==-1;
plotted=false(size(isbg));
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  id=t.id;
  loc=t.position;
  vel=t.velocity;
  fprintf('%s\n', t.tostring());
  color=colors(mod(id-1,size(colors,1))+1,:);
  % plot(t.updatedLoc(1),t.updatedLoc(2),['+',color]);
  plot(t.position(1),t.position(2),'x','Color',color);
  plot(t.legs(:,1),t.legs(:,2),'o','Color',color);
  plot(t.prevlegs(:,1),t.prevlegs(:,2),'+','Color',color);
  if args.showhits
    plot(xy(t.scanpts{1},1),xy(t.scanpts{1},2),'<','Color',color);
    plot(xy(t.scanpts{2},1),xy(t.scanpts{2},2),'>','Color',color);
  end
  plotted([t.scanpts{1};t.scanpts{2}])=true;
  for l=1:2
    % Draw legs
    leg=t.legs(l,:);
    angle=-pi:pi/20:pi;
    [x,y]=pol2cart(angle,t.legdiam/2);
    x=x+leg(1);
    y=y+leg(2);
    plot(x,y,'Color',color);
  end
end
if sum(~plotted)>0
  sel=~plotted & ~isbg;
  if sum(sel)>0
    fprintf('%d target points not matched to tracks\n', sum(sel));
  end
  if args.showhits
    plot(xy(sel,1),xy(sel,2),'.r');
    plot(xy(sel,1),xy(sel,2),'.k');
  end
end

c=axis;

bxy=[];
vxy=[];
divergence=0.0047;
for i=1:length(vis.angle)
  if (bg.freq(1,i)<0.01) || abs(vis.range(i)-bg.range(1,i))>0.1
    vxy(end+1,:)=range2xy(vis.angle(i)-divergence+vis.rotation*pi/180,vis.range(i))+vis.origin;
    vxy(end+1,:)=range2xy(vis.angle(i)+divergence+vis.rotation*pi/180,vis.range(i))+vis.origin;
    vxy(end+1,:)=nan;
  end
  if (bg.freq(1,i)>0.01)
    bxy(end+1,:)=range2xy(bg.angle(i)-divergence+vis.rotation*pi/180,bg.range(1,i))+vis.origin;
    bxy(end+1,:)=range2xy(bg.angle(i)+divergence+vis.rotation*pi/180,bg.range(1,i))+vis.origin;
    bxy(end+1,:)=nan;
  end
end
plot(vxy(:,1),vxy(:,2),'g');
plot(bxy(:,1),bxy(:,2),'k');

% Plot the LIDAR position
plot(snap.vis.origin(1),snap.vis.origin(2),'mx','MarkerSize',20);

axis image;
xyt=xy(~isbg,:);

if args.crop
  ctr=(c([1,3])+c([2,4]))/2;
  sz=max(ceil(2*c([2,4]))-floor(2*c([1,3])))/2;
  newc=[ctr(1)-sz/2,ctr(1)+sz/2,ctr(2)-sz/2,ctr(2)+sz/2];
  axis(newc);  % Zoom to ROI
else
  axis([-args.maxrange,args.maxrange,-1,args.maxrange]);
end

% Plot info along the left
c=axis;
lmargin=c(1)+(c(2)-c(1))/40;
baseline=c(4)-(c(4)-c(3))/40;
lastline=c(3)+(c(4)-c(3))/20;
skip=(c(4)-c(3))/30;

h=text(lmargin,lastline,sprintf('%s',datestr(snap.vis.acquired-7/24,'yyyy-mm-dd HH:MM:SS.FFF')));
set(h,'FontSize',14);
lastline=lastline+skip;
h=text(lmargin,lastline,sprintf('Frame: %d Unit: %d',snap.vis.frame, snap.vis.unit));
set(h,'FontSize',18);
lastline=lastline+skip;

for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  color=colors(mod(t.id-1,size(colors,1))+1,:);
  if t.consecutiveInvisibleCount>0
    invcode=sprintf(' I=%d',t.consecutiveInvisibleCount);
  else
    invcode='';
  end
  h=text(lmargin,baseline,'O');
  set(h,'Color',color);
  h=text(lmargin,baseline,sprintf('    P%d: A=%d%s L=%.0f',t.id,t.age,invcode,t.maxlike));
  set(h,'FontSize',14);
  baseline=baseline-skip;
end

end
