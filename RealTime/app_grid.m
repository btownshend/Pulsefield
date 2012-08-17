% Hotspots app
function info=app_grid(info,op)
debug=true;
if ~isfield(info,'grid')|| strcmp(op,'start')
  minx=min(info.p.layout.active(:,1))+0.1;
  maxx=max(info.p.layout.active(:,1))-0.5;
  miny=min(info.p.layout.active(:,2))+0.1;
  maxy=max(info.p.layout.active(:,2))-0.1;
  nx=8;
  ny=9;
  dx=(maxx-minx)/nx;
  dy=(maxy-miny)/ny;
  hysteresis=min([dx,dy])/5;
  ind=1;position=[]; pitch=[];
  for i=nx:-1:1
    for j=1:ny
      pos=[minx+(i-0.5)*dx,miny+(j-0.5)*dy];
      if inpolygon(pos(1),pos(2),info.p.layout.active(:,1),info.p.layout.active(:,2))
        position(end+1,:)=pos;
      end
      ind=ind+1;
    end
  end
  cell=1:size(position,1);
  pitch=cell+round(64-mean(cell));
  info.grid=struct('maxcliptime',5.0, 'hysteresis',hysteresis,'minx',minx,'maxx',maxx,'miny',miny,'maxy',maxy,'nx',nx,'ny',ny,'position',position,'pitch',pitch,'last',struct('cell',{},'id',{},'triggertime',{},'offtime',{}));
  % Initialize interface
  for channel=1:16
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
  end    
end

if strcmp(op,'stop')
  % Turn off everything
  for channel=1:16
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
  end
end

if strcmp(op,'plot')
  setfig('grid');
  clf;
  plotlayout(info.p.layout,0);
  hold on;
  dx=(info.grid.maxx-info.grid.minx)/info.grid.nx;
  dy=(info.grid.maxy-info.grid.miny)/info.grid.ny;
  for i=0:info.grid.nx
    plot((info.grid.minx+i*dx)*[1 1],[info.grid.miny,info.grid.maxy],'g');
    if i>0
      plot((info.grid.minx+i*dx)*[1 1]-info.grid.hysteresis,[info.grid.miny,info.grid.maxy],'c:');
    end
    if i<info.grid.nx
      plot((info.grid.minx+i*dx)*[1 1]+info.grid.hysteresis,[info.grid.miny,info.grid.maxy],'c:');
    end
  end
  for j=0:info.grid.ny
    plot([info.grid.minx,info.grid.maxx],(info.grid.miny+j*dy)*[1 1],'g');
    if j>0
      plot([info.grid.minx,info.grid.maxx],(info.grid.miny+j*dy)*[1 1]-info.grid.hysteresis,'c:');
    end
    if j<info.grid.ny
      plot([info.grid.minx,info.grid.maxx],(info.grid.miny+j*dy)*[1 1]+info.grid.hysteresis,'c:');
    end
  end
  for i=1:size(info.grid.position,1)
    text(info.grid.position(i,1)-dx/4,info.grid.position(i,2),sprintf('%d',i));
  end
  title(sprintf('Grid layout (%d wide by %d high with %d cells) (%.2fm x %.2fm)',info.grid.nx, info.grid.ny, size(info.grid.position,1),dx,dy));
  pause(0.1);
end

if ~strcmp(op,'update')
  return;
end

for i=1:length(info.updates)
  id=info.updates(i);
  ids=[info.snap.hypo.id];
  pos=info.snap.hypo(ids==id).pos;
  dist=sqrt((pos(1)-info.grid.position(:,1)).^2 + (pos(2)-info.grid.position(:,2)).^2);
  [newdist,newcell]=min(dist);
  last=info.grid.last([info.grid.last.id]==id);
  if isempty(last)       % Since this is an update, there was already an entry
    % This could occur if we switched to app_grid in midstream so there were already entries not processed here
    fprintf('app_grid: len(last.cell)~=1, last.cell=%s\n', sprintf('%d ', last.cell));
    info.grid.last=[info.grid.last,struct('cell',newcell,'id',id,'triggertime',now,'offtime',now)];
    % No notes yet, only if they move to a new grid
    continue;
  end
  if length(last)==1 && last.cell==newcell
    % No change
    continue;
  end
  % Check if we've moved cleanly to another grid (to avoid oscillating near boundary)
  lastdist=norm(info.grid.position(last.cell,:)-pos);
  if (lastdist-newdist)/2 < info.grid.hysteresis
    % Not far enough
    fprintf('Sticking at %d (d=%.2f) instead of %d (%.2f)\n',  last.cell, lastdist, newcell, newdist);
    newcell=last.cell;
  end
  if newcell~=last.cell
    info.grid.last([info.grid.last.id]==id)=struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan);
    channel=id2channel(info,id);
    lastpitch=info.grid.pitch(last.cell);
    newpitch=info.grid.pitch(newcell);
    if info.max
      oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(lastpitch),int32(channel)});
      oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
    end
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{sprintf('%d:%d(%s)',newcell,newpitch,midinotename(newpitch))});
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'green'});
    if info.ableton
      pan=pos(2)/max(abs(info.p.layout.active(:,2)));
      cliptrigger(channel-1,newcell,1.0,pan);
    end
    if debug
      fprintf('ID %d (channel %d) moved from grid %d (%.2f) to grid %d (%.2f)\n', id, channel, last.cell, lastdist, newcell, newdist);
    end
  end
end

for i=1:length(info.exits)
  id=info.exits(i);
  last=info.grid.last([info.grid.last.id]==id);
  lastpitch=info.grid.pitch(last.cell);
  channel=id2channel(info,id);
  oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
  oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
  if info.max
    oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(lastpitch),int32(channel)});
  end
  if info.ableton
    oscmsgout('AL','/live/stop/track',{int32(channel-1)});
  end
  info.grid.last=info.grid.last([info.grid.last.id]~=id);
  if debug
    fprintf('ID %d exitted from grid %d\n', id, last.cell);
  end
end

for i=1:length(info.entries)
  id=info.entries(i);
  if any([info.grid.last.id]==id)
    fprintf('Already have id %d inside\n', id);
    continue;
  end
  ids=[info.snap.hypo.id];
  pos=info.snap.hypo(ids==id).pos;
  dist=sqrt((pos(1)-info.grid.position(:,1)).^2 + (pos(2)-info.grid.position(:,2)).^2);
  [newdist,newcell]=min(dist);
  channel=id2channel(info,id);
  newpitch=info.grid.pitch(newcell);
  oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{sprintf('%d:%d(%s)',newcell,newpitch,midinotename(newpitch))});
  oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'green'});
  if info.max
    oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
  end
  if info.ableton
    pan=pos(2)/max(abs(info.p.layout.active(:,2)));
    cliptrigger(channel-1,newcell,1.0,pan);
  end
  info.grid.last=[info.grid.last,struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan)];
  if debug
    fprintf('ID %d entered to grid %d\n', id, newcell);
  end
end

if length(info.exits)>0 && isempty(info.entries) && isempty(info.updates)
  % Last person exitted
  fprintf('Last person exitted grid\n');
  if info.ableton
    oscmsgout('AL','/live/stop',{});
  end
end

if info.ableton
  % Terminate clips after a certain time
  for i=1:length(info.grid.last)
    l=info.grid.last(i);
    if isnan(l.offtime) && (now-l.triggertime)*24*3600>info.grid.maxcliptime
      channel=id2channel(info,l.id);
      oscmsgout('AL','/live/stop/track',{int32(channel-1)});
      oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
      info.grid.last(i).offtime=now;
      if debug
        fprintf('Ended clip on track %d\n', channel);
      end
    end
  end
end
      

% Trigger a clip on given track in Ableton Live (via LiveOSC)
% Track is track number starts with 0
% Clip is one in the scene number in AL (starts with 0)
% volume is 0.0-1.0,  pan is -1.0 to 1.0
function cliptrigger(track,clip,volume,pan)
if nargin<3
  volume=1.0;
end
if nargin<4
  pan=0.0;
end
oscmsgout('AL','/live/play/clip',{int32(track),int32(clip)});
oscmsgout('AL','/live/volume',{int32(track),volume});
oscmsgout('AL','/live/pan',{int32(track),pan});

% Map grid cell to AL scene
function scene=cell2scene(info,cell)
