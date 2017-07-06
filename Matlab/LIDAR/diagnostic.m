% Diagnostic plots/output
function diagnostic(snap,varargin)
defaults=struct('trackid',[],...   % Only show these trackids
                'frames',[],...
                'minage',1,...
                'debug',false,...
                'other',[],...
                'plotvar',false...
                );
args=processargs(defaults,varargin);
if isempty(args.other)
  args.other={'legsep','speed'};
end
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
  trackedBy=ones(1,length(idpresent));
  for k=1:length(idpresent)
    if idpresent(k) 
      trackIndex=find(arrayfun(@(y) y.id, snap(k).tracker.tracks)==ids(i));
      idpresent(k)=~isempty(snap(k).tracker.tracks(trackIndex).position);
      % Determine which LIDAR was tracking
      if isfield(snap(k).tracker.tracks,'trackedBy')
        trackedBy(k)=snap(k).tracker.tracks(trackIndex).trackedBy;
      end
    end
  end
  idtmp=nan(1,length(snap));
  idtmp(idpresent)=ids(i);
  color1=colors(mod(ids(i)-1,length(colors))+1);
  if ismember(ids(i),args.trackid) || isempty(args.trackid)
    % Show ID number + LIDARtracking/10
    plot(frame,idtmp+trackedBy/10,[color1,'-']);
  else
    plot(frame,idtmp+trackedBy/10,[color1,':']);
  end
  hold on;
end
c=axis;
c(3)=c(3)-0.1;c(4)=c(4)+0.1;
axis(c);
ylabel('ID Presence');

for i=1:length(ids)
  id=ids(i);
  color1=colors(mod(id-1,length(colors))+1);
  if length(args.trackid)==1
    color2=colors(mod(id,length(colors))+1);
  else
    color2=color1;  % Same color for both legs if more than 1 ID
  end
  
  if ~isempty(args.trackid) && ~ismember(id,args.trackid)
    continue;
  end
  loc=nan(length(snap),2,2);
  leftness=nan(length(snap),2);
  predloc=nan(length(snap),2,2);
  posvar=nan(length(snap),2);
  legvel=nan(length(snap),2,2);
  vel=nan(length(snap),2);
  diam=nan(length(snap),1);
  vis=false(length(snap),2);
  scanpts=nan(length(snap),2);
  nsteps=nan(length(snap),2);   % Number of prediction frames since last fix
  activeLIDAR=true(length(snap),1);   % Active LIDAR?
  for j=1:length(snap)
    sel=arrayfun(@(z) z.id, snap(j).tracker.tracks)==id;
    if sum(sel)>0
      loc(j,:,:)=snap(j).tracker.tracks(sel).legs;
      predloc(j,:,:)=snap(j).tracker.tracks(sel).predictedlegs;
      posvar(j,:)=snap(j).tracker.tracks(sel).posvar;
      vel(j,:)=snap(j).tracker.tracks(sel).velocity;
      legvel(j,:,:)=snap(j).tracker.tracks(sel).legvelocity;
      leftness(j,:)=snap(j).tracker.tracks(sel).leftness;
      diam(j)=snap(j).tracker.tracks(sel).legdiam;
      if isfield(snap(j).vis,'unit')
        activeLIDAR(j)=snap(j).tracker.tracks(sel).trackedBy==snap(j).vis.unit;
      end
      if isprop(snap(j).tracker.tracks(sel),'scanpts')
        vis(j,1)=~isempty(snap(j).tracker.tracks(sel).scanpts{1});
        vis(j,2)=~isempty(snap(j).tracker.tracks(sel).scanpts{2});
        scanpts(j,1)=length(snap(j).tracker.tracks(sel).scanpts{1});
        scanpts(j,2)=length(snap(j).tracker.tracks(sel).scanpts{2});
      else
        vis(j,:)=[snap(j).tracker.tracks(sel).legclasses]>0;
      end
      if (j>1)
        for k=1:2
          if vis(j-1,k)
            nsteps(j,k)=frame(j)-frame(j-1);
          else
            nsteps(j,k)=nsteps(j-1,k)+(frame(j)-frame(j-1));
            nsteps(j-1,k)=nan;
          end
        end
      end
    end
  end
  subplot(231);
  plot(loc(:,1,1),loc(:,1,2),[color1,'-']);
  hold on;
  plot(loc(:,2,1),loc(:,2,2),[color2,'-']);
  plot(loc(vis(:,1),1,1),loc(vis(:,1),1,2),[color1,'.'],'MarkerSize',10);
  plot(loc(vis(:,2),2,1),loc(vis(:,2),2,2),[color2,'.'],'MarkerSize',10);
  % Invisible ones
  plot(loc(~vis(:,1),1,1),loc(~vis(:,1),1,2),[color1,'o']);
  plot(loc(~vis(:,2),2,1),loc(~vis(:,2),2,2),[color2,'o']);
  % Connector lines every 10 frames
  conn=find(mod(frame,5)==0);
  for ci=1:length(conn)
    plot([loc(conn(ci),:,1)],[loc(conn(ci),:,2)],'Color',[1,1,1]*0.9);
  end
  axis equal
  c=axis;
  
  subplot(234);
  plot(loc(:,1,1),frame,[color1,'-']);
  hold on;
  plot(loc(:,2,1),frame,[color2,'-']);
  if args.plotvar
    for ii=1:size(posvar,1)
      plot(loc(ii,1,1)+sqrt(posvar(ii,1))*[-1,1],frame(ii)+[-0.2,-0.2],[color1,':']);
      plot(loc(ii,2,1)+sqrt(posvar(ii,2))*[-1,1],frame(ii)+[0.2,0.2],[color2,':']);
    end
  end
  % Visible points
  plot(loc(vis(:,1)&activeLIDAR,1,1),frame(vis(:,1)&activeLIDAR),[color1,'.'],'MarkerSize',10);
  plot(loc(vis(:,2)&activeLIDAR,2,1),frame(vis(:,2)&activeLIDAR),[color2,'.'],'MarkerSize',10);
  cx=axis;
  cx(1:2)=c(1:2);
  axis(cx);
  ylabel('Frame');
  xlabel('X Position');
  title('X Position');
  
  subplot(232)
  plot(frame,loc(:,1,2),[color1,'-']);
  hold on;
  plot(frame,loc(:,2,2),[color2,'-']);
  if args.plotvar
    for ii=1:size(posvar,1)
      plot(frame(ii)+[-0.2,-0.2],loc(ii,1,2)+sqrt(posvar(ii,1))*[-1,1],[color1,':']);
      plot(frame(ii)+[0.2,0.2],loc(ii,2,2)+sqrt(posvar(ii,2))*[-1,1],[color2,':']);
    end
  end
  plot(frame(vis(:,1)&activeLIDAR),loc(vis(:,1)&activeLIDAR,1,2),[color1,'.'],'MarkerSize',10);
  plot(frame(vis(:,2)&activeLIDAR),loc(vis(:,2)&activeLIDAR,2,2),[color2,'.'],'MarkerSize',10);
  cy=axis;
  cy(3:4)=c(3:4);
  axis(cy);
  xlabel('Frame');
  ylabel('Y Position');
  title('Y Position');

  if ~iscell(args.other)
    args.other={args.other};
  end
  for oth=1:min(2,length(args.other))
    if oth==1
      subplot(233);
    else
      subplot(236);
    end
    if strcmp(args.other(oth),'speed')
      [~,spd]=cart2pol(vel(:,1),vel(:,2));
      plot(frame,spd,[color1,'-']);
      hold on;
      plot(frame(vis(:,1)&vis(:,2)),spd(vis(:,1)&vis(:,2)),[color1,'.']);
      xlabel('Frame');
      title('Total Speed');
    elseif strcmp(args.other(oth),'legspeed')
      [~,spd1]=cart2pol(legvel(:,1,1),legvel(:,1,2));
      [~,spd2]=cart2pol(legvel(:,2,1),legvel(:,2,2));
      plot(frame,spd1,[color1,'--']);
      hold on;
      plot(frame,spd2,[color2,'-']);
      plot(frame(vis(:,1)),spd1(vis(:,1)),[color1,'.']);
      plot(frame(vis(:,2)),spd2(vis(:,2)),[color2,'.']);
      xlabel('Frame');
      title('Leg Speed');
    elseif strcmp(args.other(oth),'predict')
      pstd=sqrt((loc(:,:,1)-predloc(:,:,1)).^2+(loc(:,:,2)-predloc(:,:,2)).^2);
      pstd(all(predloc==0,3))=nan;   % Initial prediction
      plot(frame,pstd(:,1),[color1,'-']);
      hold on;
      plot(frame,pstd(:,2),[color2,'-']);
      % Visible points
      plot(frame(vis(:,1)),pstd(vis(:,1),1),[color1,'.'],'MarkerSize',10);
      plot(frame(vis(:,2)),pstd(vis(:,2),2),[color2,'.'],'MarkerSize',10);
      xlabel('Frame');
      ylabel('Prediction Error (m)');
      title('Prediction error');
    elseif strcmp(args.other(oth),'predbar')
      pstd=sqrt((loc(:,:,1)-predloc(:,:,1)).^2+(loc(:,:,2)-predloc(:,:,2)).^2);
      pstd(all(predloc==0,3))=nan;   % Initial prediction
      steprange=1:20;
      mn=zeros(1,length(steprange));
      for k=steprange
        if (sum(nsteps(:)==k)>=5)
          mn(k)=sqrt(nanmean(pstd(nsteps(:)==k).^2));
        end
      end
      bar(steprange,mn);
      xlabel('Frames since prior fix');
      ylabel('Mean Prediction Error (m)');
      title('Prediction error vs frames since prior fix');
    elseif strcmp(args.other(oth),'predvssteps')
      pstd=sqrt((loc(:,:,1)-predloc(:,:,1)).^2+(loc(:,:,2)-predloc(:,:,2)).^2);
      pstd(all(predloc==0,3))=nan;   % Initial prediction
      plot(nsteps(:,1)+rand(size(nsteps,1),1)-0.5,pstd(:,1),[color1,'.']);
      hold on;
      plot(nsteps(:,2)+rand(size(nsteps,1),1)-0.5,pstd(:,2),[color2,'.']);
      xlabel('Frames since prior fix');
      ylabel('Prediction Error (m)');
      title('Predicted Error vs frames since fix');
    elseif strcmp(args.other(oth),'predvsposvar')
      pstd=sqrt((loc(:,:,1)-predloc(:,:,1)).^2+(loc(:,:,2)-predloc(:,:,2)).^2);
      pstd(all(predloc==0,3))=nan;   % Initial prediction
      plot(pstd(:,1),sqrt(posvar(:,1)),[color1,'.']);
      hold on;
      plot(pstd(:,2),sqrt(posvar(:,2)),[color2,'.']);
      axis equal
      ylabel('sqrt(posvar Estimate) (m)');
      xlabel('Prediction Error (m)');
      title('Predicted vs Observed error');
    elseif strcmp(args.other(oth),'scanpts')
      plot(frame,scanpts(:,1),[color1,'-']);
      hold on;
      plot(frame,scanpts(:,2),[color2,'-']);
      plot(frame(activeLIDAR),scanpts(activeLIDAR,1),[color1,'.'],'MarkerSize',10);
      plot(frame(activeLIDAR),scanpts(activeLIDAR,2),[color2,'.'],'MarkerSize',10);
      xlabel('Frame');
      ylabel('Number of scan pts');
      title('Scan pts');
    elseif strcmp(args.other(oth),'nsteps')
      plot(frame,nsteps(:,1),[color1,'-']);
      hold on;
      plot(frame,nsteps(:,2),[color2,'-']);
      xlabel('Frame');
      ylabel('Frames since prior fix');
      title('Fix Intervals');
    elseif strcmp(args.other(oth),'leftness')
      plot(frame,leftness(:,1),[color1,'-']);
      hold on;
      plot(frame,leftness(:,2),[color2,'-']);
      xlabel('Frame');
      ylabel('Leftness');
      title('Leftness');
    elseif strcmp(args.other(oth),'legsep')
      sep=loc(:,1,:)-loc(:,2,:);
      plot(frame,squeeze(sqrt(sep(:,:,1).^2+sep(:,:,2).^2)),[color1,'-']);
      xlabel('Frame');
      title('Leg Sep');
    elseif strcmp(args.other(oth),'legdiam')
      plot(frame,diam,[color1,'-']);
      xlabel('Frame');
      ylabel('Diameter');
      title('Leg Diam');
    end
  end
end



