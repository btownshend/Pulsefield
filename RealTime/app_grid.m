% Hotspots app
function info=app_grid(p,info,op)
debug=true;
if ~isfield(info,'grid')|| strcmp(op,'start')
  minx=min(p.layout.active(:,1))+0.1;
  maxx=max(p.layout.active(:,1))-0.5;
  miny=min(p.layout.active(:,2))+0.1;
  maxy=max(p.layout.active(:,2))-0.1;
  nx=8;
  ny=9;
  dx=(maxx-minx)/nx;
  dy=(maxy-miny)/ny;
  hysteresis=min([dx,dy])/5;
  ind=1;position=[]; pitch=[];
  for i=nx:-1:1
    for j=1:ny
      pos=[minx+(i-0.5)*dx,miny+(j-0.5)*dy];
      if inpolygon(pos(1),pos(2),p.layout.active(:,1),p.layout.active(:,2))
        position(end+1,:)=pos;
      end
      ind=ind+1;
    end
  end
  cell=1:size(position,1);
  pitch=cell+round(64-mean(cell));
  info.grid=struct('maxcliptime',5.0, 'hysteresis',hysteresis,'minx',minx,'maxx',maxx,'miny',miny,'maxy',maxy,'nx',nx,'ny',ny,'position',position,'pitch',pitch,'active',struct('cell',{},'id',{},'triggertime',{},'offtime',{}),'trackspersong',8,'song',4,'tempos',[100, 108, 72, 120, 111, 93, 93, 108, 120, 120, 120,120]);
  % Initialize interface
  for channel=1:8
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
    oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{''});
  end  
  % Make sure all Ableton channels are off
  oscmsgout('AL','/live/stop',{});
  for i=1:info.cm.numchannels()
    oscmsgout('AL','/live/stop/track',{int32(i-1)});
  end
end

if strcmp(op,'stop')
  % Turn off everything
  oscmsgout('AL','/live/stop',{});
  for channel=1:8
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
    oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{''});
  end
  for track=1:info.al.numtracks()-1
    % Make sure all Ableton channels are off
    oscmsgout('AL','/live/stop/track',{int32(track)});
  end
end

if strcmp(op,'plot')
  setfig('grid');
  clf;
  plotlayout(p.layout,0);
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

% Read any Ableton updates
info.al.refresh();
info.grid.numsongs=info.al.numtracks()/info.grid.trackspersong;
alchannels=0;
for i=(0:7)+(info.grid.song-1)*8;
  if ~strcmp(info.al.gettrackname(i),'Empty')
    alchannels=alchannels+1;
  end
end

if (alchannels ~= info.cm.numchannels() && alchannels>0)
  fprintf('Ableton has %d (->%d) tracks, but using a %d channel mapping; changing mapping\n',info.al.numtracks(), alchannels, info.cm.numchannels());
  info.cm.setnumchannels(alchannels);
end
%fprintf('numsongs=%f, trackspersong=%d, song=%d, channels=%d\n',info.grid.numsongs, info.grid.trackspersong, info.grid.song,alchannels);
for i=1:length(info.updates)
  id=info.updates(i);
  ids=[info.snap.hypo.id];
  pos=info.snap.hypo(ids==id).pos;
  dist=sqrt((pos(1)-info.grid.position(:,1)).^2 + (pos(2)-info.grid.position(:,2)).^2);
  [newdist,newcell]=min(dist);
  active=info.grid.active([info.grid.active.id]==id);
  if isempty(active)       % Since this is an update, there was already an entry
    % This could occur if we switched to app_grid in midstream so there were already entries not processed here
    fprintf('app_grid: len(active.cell)~=1, active.cell=%s\n', sprintf('%d ', active.cell));
    info.grid.active=[info.grid.active,struct('cell',newcell,'id',id,'triggertime',now,'offtime',now)];
    % No notes yet, only if they move to a new grid
    continue;
  end
  if length(active)==1 && active.cell==newcell
    % No change
    continue;
  end
  % Check if we've moved cleanly to another grid (to avoid oscillating near boundary)
  activedist=norm(info.grid.position(active.cell,:)-pos);
  if (activedist-newdist)/2 < info.grid.hysteresis
    % Not far enough
    fprintf('Sticking at %d (d=%.2f) instead of %d (%.2f)\n',  active.cell, activedist, newcell, newdist);
    newcell=active.cell;
  end
  if newcell~=active.cell
    info.grid.active([info.grid.active.id]==id)=struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan);
    channel=info.cm.id2channel(id);
    activepitch=info.grid.pitch(active.cell);
    newpitch=info.grid.pitch(newcell);
    if info.max
      oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(activepitch),int32(channel)});
      oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
    end
    oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{sprintf('%d:%d(%s)',newcell,newpitch,midinotename(newpitch))});
    oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{sprintf('%s',info.al.getclipname(channel-1,newcell-1))});
    oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'green'});
    if info.ableton
      pan=pos(2)/max(abs(p.layout.active(:,2)));
      cliptrigger(info.al,info,channel-1,newcell-1,1.0,pan);
    end
    if debug
      fprintf('ID %d (channel %d) moved from grid %d (%.2f) to grid %d (%.2f)\n', id, channel, active.cell, activedist, newcell, newdist);
    end
  end
end

for i=1:length(info.exits)
  id=info.exits(i);
  active=info.grid.active([info.grid.active.id]==id);
  activepitch=info.grid.pitch(active.cell);
  channel=info.cm.id2channel(id);
  oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
  oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{''});
  oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'red'});
  if info.max
    oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(activepitch),int32(channel)});
  end
  if info.ableton
    track=(info.grid.song-1)*info.grid.trackspersong+channel-1;
    oscmsgout('AL','/live/stop/track',{int32(track)});
  end
  info.grid.active=info.grid.active([info.grid.active.id]~=id);
  if debug
    fprintf('ID %d exitted from grid %d\n', id, active.cell-1);
  end
end

for i=1:length(info.entries)
  id=info.entries(i);
  if any([info.grid.active.id]==id)
    fprintf('Already have id %d inside\n', id);
    continue;
  end
  ids=[info.snap.hypo.id];
  pos=info.snap.hypo(ids==id).pos;
  dist=sqrt((pos(1)-info.grid.position(:,1)).^2 + (pos(2)-info.grid.position(:,2)).^2);
  [newdist,newcell]=min(dist);
  channel=info.cm.id2channel(id);
  newpitch=info.grid.pitch(newcell);
  oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{sprintf('%d:%d(%s)',newcell,newpitch,midinotename(newpitch))});
  oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{info.al.getclipname(channel-1,newcell-1)});
  oscmsgout('TO',sprintf('/grid/channel/%d/label/color',channel),{'green'});
  if info.max
    oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
  end
  if info.ableton
    pan=pos(2)/max(abs(p.layout.active(:,2)));
    cliptrigger(info.al,info,channel-1,newcell-1,1.0,pan);
  end
  info.grid.active=[info.grid.active,struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan)];
  if debug
    fprintf('ID %d entered to grid %d\n', id, newcell);
  end
end

if length(info.exits)>0 && isempty(info.entries) && isempty(info.updates)
  % Active person exitted
  fprintf('Last person exitted grid\n');
  if info.ableton
    oscmsgout('AL','/live/stop',{});
    for track=1:info.al.numtracks()-1
      % Make sure all Ableton channels are off
      oscmsgout('AL','/live/stop/track',{int32(track)});
    end
  end
  info.grid.song=mod(info.grid.song,info.grid.numsongs)+1;
end

if info.ableton
  % Terminate clips after a certain time
  for i=1:length(info.grid.active)
    l=info.grid.active(i);
    if isnan(l.offtime) && (now-l.triggertime)*24*3600>info.grid.maxcliptime
      channel=info.cm.id2channel(l.id);
      track=(info.grid.song-1)*info.grid.trackspersong+channel-1;
      oscmsgout('AL','/live/stop/track',{int32(track)});
      oscmsgout('TO',sprintf('/grid/channel/%d/pitch',channel),{''});
      oscmsgout('TO',sprintf('/grid/channel/%d/clip',channel),{''});
      info.grid.active(i).offtime=now;
      if debug
        fprintf('Ended clip on channel %d, track %d\n', channel, track);
      end
    end
  end
end
      

% Trigger a clip on given track in Ableton Live (via LiveOSC)
% Track is track number starts with 0
% Clip is one in the scene number in AL (starts with 0)
% volume is 0.0-1.0,  pan is -1.0 to 1.0
function cliptrigger(al,info,track,clip,volume,pan)
if nargin<3
  volume=1.0;
end
if nargin<4
  pan=0.0;
end
altrack=(info.grid.song-1)*info.grid.trackspersong+track;
oscmsgout('AL','/live/tempo',{int32(info.grid.tempos(info.grid.song))});
%oscmsgout('AL','/live/volume',{int32(track),volume});
%oscmsgout('AL','/live/pan',{int32(track),pan});
nm=al.getclipname(altrack,clip);
fprintf('cliptrigger: song=%d,track=%d,altrack=%d (%s),clip=%d (%s),tempo=%d\n', info.grid.song,track, altrack, al.gettrackname(altrack), clip, nm, info.grid.tempos(info.grid.song));
if isempty(nm)
  oscmsgout('AL','/live/stop/track',{int32(altrack)});
else  
  oscmsgout('AL','/live/play/clip',{int32(altrack),int32(clip)});
end
%oscmsgout('AL','/live/play/track',{int32(altrack)});

% Map grid cell to AL scene
function scene=cell2scene(info,cell)
