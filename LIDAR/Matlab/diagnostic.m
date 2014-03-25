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

tracker=snap(end).tracker;
MAXSPECIAL=2;
colors='rgbcmk';

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
  legvel=nan(length(snap),2,2);
  vel=nan(length(snap),2);
  vis=false(length(snap),2);
  for j=1:length(snap)
    sel=arrayfun(@(z) z.id, snap(j).tracker.tracks)==id;
    if sum(sel)>0
      loc(j,:,:)=snap(j).tracker.tracks(sel).legs;
      vel(j,:)=snap(j).tracker.tracks(sel).velocity;
      legvel(j,:,:)=snap(j).tracker.tracks(sel).legvelocity;
      vis(j,:)=[snap(j).tracker.tracks(sel).legclasses]~=1;
    end
  end
  subplot(231);
  plot(loc(:,1,1),loc(:,1,2),[color,'-']);
  hold on;
  plot(loc(:,2,1),loc(:,2,2),[color,'-']);
  plot(loc(vis(:,1),1,1),loc(vis(:,1),1,2),[color,'.']);
  plot(loc(vis(:,2),2,1),loc(vis(:,2),2,2),[color,'.']);
  % Invisible ones
  plot(loc(~vis(:,1),1,1),loc(~vis(:,1),1,2),[color,'o']);
  plot(loc(~vis(:,2),2,1),loc(~vis(:,2),2,2),[color,'o']);
  axis equal
  c=axis;
  
  subplot(234);
  plot(loc(:,1,1),frame,[color,'-']);
  hold on;
  plot(loc(:,2,1),frame,[color,'-']);
  % Visible points
  plot(loc(vis(:,1),1,1),frame(vis(:,1)),[color,'.']);
  plot(loc(vis(:,2),2,1),frame(vis(:,2)),[color,'.']);
  cx=axis;
  cx(1:2)=c(1:2);
  axis(cx);
  ylabel('Frame');
  xlabel('X Position');
  title('X Position');
  
  subplot(232)
  plot(frame,loc(:,1,2),[color,'-']);
  hold on;
  plot(frame,loc(:,2,2),[color,'-']);
  plot(frame(vis(:,1)),loc(vis(:,1),1,2),[color,'.']);
  plot(frame(vis(:,2)),loc(vis(:,2),2,2),[color,'.']);
  cy=axis;
  cy(3:4)=c(3:4);
  axis(cy);
  xlabel('Frame');
  ylabel('Y Position');
  title('Y Position');

  subplot(233);
  [heading,~]=cart2pol(vel(:,1),vel(:,2));
  plot(frame,heading*180/pi,[color,'.-']);
  hold on;
  xlabel('Frame');
  title('Overall Heading');
  
  subplot(236)
  [~,spd1]=cart2pol(legvel(:,1,1),legvel(:,1,2));
  [~,spd2]=cart2pol(legvel(:,2,1),legvel(:,2,2));
  plot(frame,spd1,[color,'-']);
  hold on;
  plot(frame,spd2,[color,'-']);
  plot(frame(vis(:,1)),spd1(vis(:,1)),[color,'.']);
  plot(frame(vis(:,2)),spd2(vis(:,2)),[color,'.']);
  xlabel('Frame');
  title('Leg Speed');
end


