% Diagnostic plots/output
function diagnostic(snap,varargin)
defaults=struct('trackid',[],...   % Only show these trackids
                'frames',[],...
                'minage',1,...
                'debug',false...
                );
args=processargs(defaults,varargin);

if isempty(args.trackid) & args.minage>1
  % Only show tracks that were visible for minage samples
  tid=[];
  for i=1:length(snap)
    if ~isempty(snap(i).tracker.tracks)
      tid=[tid,snap(i).tracker.tracks(arrayfun(@(z) z.age, snap(i).tracker.tracks)>=args.minage).id];
    end
  end
  args.trackid=unique(tid);
  fprintf('Showing track IDS %s\n', sprintf('%d ',args.trackid));
end

if ~isempty(args.frames)
  frame=arrayfun(@(z) z.vis.frame, snap);
  i1=find(frame>=args.frames(1),1);
  i2=find(frame<=args.frames(2),1,'last');
  snap=snap(i1:i2);
  fprintf('Showing snap(%d:%d)\n', i1, i2);
end

frame=arrayfun(@(z) z.vis.frame,snap);

bg=snap(end).bg;
vis=snap(end).vis;
tracker=snap(end).tracker;
MAXSPECIAL=2;
fprintf('\n');
setfig('diagnostic');clf;
hold on;

xy=range2xy(vis.angle,vis.range);
bxy=range2xy(bg.angle,bg.range(1,:));

colors='gbcymk';
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
  cnum=t.legclasses;
  cnum(cnum==1)=nan;
  %fprintf('class=(%d,%d), npts=(%d,%d)\n',cnum,sum(vis.class==cnum(1)),sum(vis.class==cnum(2)));
  lsel=vis.class==cnum(1);
  rsel=vis.class==cnum(2);
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

if length(snap)>1
  % Plot locations
  setfig('diagnostic-tracks');clf;
  ids=[];
  for i=1:length(snap)
    ids=unique([ids,arrayfun(@(z) z.id, snap(i).tracker.tracks)]);
  end
  subplot(235);
  for i=1:length(ids)
    idpresent=arrayfun(@(z) ismember(ids(i),arrayfun(@(y) y.id, z.tracker.tracks)), snap);
    % Only present if there was also a measurement 
    for k=1:length(idpresent)
      if idpresent(k) 
        idpresent(k)=~isempty(snap(k).tracker.tracks(arrayfun(@(y) y.id, snap(k).tracker.tracks)==ids(i)).position);
      end
    end
    idtmp=nan(1,length(snap));
    idtmp(idpresent)=ids(i);
    if ismember(ids(i),args.trackid)
      plot(frame,idtmp,'g');
    else
      plot(frame,idtmp,'r');
    end
    hold on;
  end
  c=axis;
  c(3)=c(3)-0.1;c(4)=c(4)+0.1;
  axis(c);
  ylabel('ID Presence');
  
  for i=1:length(ids)
    id=ids(i);
    color=colors(mod(id-1,length(colors))+1);
    if ~isempty(args.trackid) && ~ismember(id,args.trackid)
      continue;
    end
    loc=nan(length(snap),2,2);
    vel=nan(length(snap),2);
    for j=1:length(snap)
      sel=arrayfun(@(z) z.id, snap(j).tracker.tracks)==id;
      if sum(sel)>0
        loc(j,:,:)=snap(j).tracker.tracks(sel).legs;
        vel(j,:)=snap(j).tracker.tracks(sel).velocity;
      end
    end
    subplot(231);
    plot(loc(:,1,1),loc(:,1,2),[color,'.-']);
    hold on;
    plot(loc(:,2,1),loc(:,2,2),[color,'.-']);
    axis equal
    c=axis;
    
    subplot(234);
    plot(loc(:,1,1),frame,[color,'.-']);
    hold on;
    plot(loc(:,2,1),frame,[color,'.-']);
    cx=axis;
    cx(1:2)=c(1:2);
    axis(cx);
    ylabel('Frame');
    xlabel('X Position');
    title('X Position');
    
    subplot(232)
    plot(frame,loc(:,1,2),[color,'.-']);
    hold on;
    plot(frame,loc(:,2,2),[color,'.-']);
    cy=axis;
    cy(3:4)=c(3:4);
    axis(cy);
    xlabel('Frame');
    ylabel('Y Position');
    title('Y Position');

    subplot(233);
    [heading,speed]=cart2pol(vel(:,1),vel(:,2));
    plot(frame,heading*180/pi,[color,'.-']);
    hold on;
    xlabel('Frame');
    title('Heading');
    
    subplot(236)
    plot(frame,speed,[color,'.-']);
    hold on;
    xlabel('Frame');
    title('Speed');

  end
end

      
