function plotlegsep(snap,varargin) 
defaults=struct('trackid',[]...   % Only show these trackids
                );
args=processargs(defaults,varargin);

colors='rgbcmk';
frame=arrayfun(@(z) z.vis.frame, snap);
ids=[];
for i=1:length(snap)
  ids=unique([ids,arrayfun(@(z) z.id, snap(i).tracker.tracks)]);
end
setfig('legsep');clf;
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
      if isprop(snap(j).tracker.tracks(sel),'scanpts')
        vis(j,:)=~isempty(snap(j).tracker.tracks(sel).scanpts{1})||~isempty(snap(j).tracker.tracks(sel).scanpts{2});
      else
        vis(j,:)=[snap(j).tracker.tracks(sel).legclasses]>0;
      end
    end
  end
  delta=squeeze(loc(:,1,:)-loc(:,2,:));
  dist=sqrt(delta(:,1).^2+delta(:,2).^2);
  subplot(211);
  plot(frame,dist,[color,'-']);
  hold on;
  subplot(212);
  plot(frame(2:end),diff(dist),[color,'-']);
  hold on;
end
subplot(211);
xlabel('Frame');
ylabel('Leg Sep (m)');
subplot(212);
xlabel('Frame');
ylabel('Leg Sep Change (m)');
keyboard


