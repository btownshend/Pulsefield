% Hotspots app
function info=app_grid(info,op)
debug=true;
if ~isfield(info,'grid')|| strcmp(op,'start')
  minx=min(info.p.layout.active(:,1))+0.1;
  maxx=max(info.p.layout.active(:,1))-0.5;
  miny=min(info.p.layout.active(:,2))+0.1;
  maxy=max(info.p.layout.active(:,2))-0.1;
  nx=9;
  ny=12;
  dx=(maxx-minx)/nx;
  dy=(maxy-miny)/ny;
  hysteresis=min([dx,dy])/5;
  setfig('grid');
  clf;
  plotlayout(info.p.layout,0);
  hold on;
  for i=0:nx
    plot((minx+i*dx)*[1 1],[miny,maxy],'g');
    if i>0
      plot((minx+i*dx)*[1 1]-hysteresis,[miny,maxy],'c:');
    end
    if i<nx
      plot((minx+i*dx)*[1 1]+hysteresis,[miny,maxy],'c:');
    end
  end
  for j=0:ny
    plot([minx,maxx],(miny+j*dy)*[1 1],'g');
    if j>0
      plot([minx,maxx],(miny+j*dy)*[1 1]-hysteresis,'c:');
    end
    if j<ny
      plot([minx,maxx],(miny+j*dy)*[1 1]+hysteresis,'c:');
    end
  end
  ind=1;position=[]; pitch=[];
  for i=1:nx
    for j=1:ny
      pos=[minx+(i-0.5)*dx,miny+(j-0.5)*dy];
      if inpolygon(pos(1),pos(2),info.p.layout.active(:,1),info.p.layout.active(:,2))
        position(end+1,:)=pos;
        pitch(end+1)=ind;
      end
      ind=ind+1;
    end
  end
  %pitch=pitch+round(64-mean(pitch));
  for i=1:length(pitch)
    text(position(i,1)-dx/5,position(i,2),sprintf('%d',pitch(i)));
  end
  title(sprintf('Grid layout (%d wide by %d high with %d cells) (%.2fm x %.2fm)',nx, ny, length(pitch),dx,dy));
  info.grid=struct('hysteresis',hysteresis,'minx',minx,'maxx',maxx,'miny',miny,'maxy',maxy,'nx',nx,'ny',ny,'position',position,'pitch',pitch,'last',struct('id',[],'cell',[]));
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
  lastcell=info.grid.last.cell(info.grid.last.id==id);
  if length(lastcell)~=1      % Since this is an update, there was already an entry
    fprintf('Assertion failed: len(lastcell)~=1, lastcell=%s\n', sprintf('%d ', lastcell));
    keyboard
  end
  if lastcell==newcell
    % No change
    continue;
  end
  % Check if we've moved cleanly to another grid (to avoid oscillating near boundary)
  lastdist=norm(info.grid.position(lastcell,:)-pos);
  if (lastdist-newdist)/2 < info.grid.hysteresis
    % Not far enough
    fprintf('Sticking at %d (d=%.2f) instead of %d (%.2f)\n',  info.grid.pitch(lastcell), lastdist, info.grid.pitch(newcell), newdist);
    newcell=lastcell;
  end
  if newcell~=lastcell
    info.grid.last.cell(info.grid.last.id==id)=newcell;
    if info.multichannel
      channel=mod(i-1,6)+1;
    else
      channel=1;
    end
    lastpitch=info.grid.pitch(lastcell);
    newpitch=info.grid.pitch(newcell);
    oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(lastpitch),int32(channel)});
    oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
    oscmsgout('TO','/pf/grid/note',{int32(id),int32(newpitch)});
    if debug
      fprintf('ID %d moved from grid %d (%.2f) to grid %d (%.2f)\n', id, lastpitch, lastdist, newpitch, newdist);
    end
  end
end

for i=1:length(info.exits)
  id=info.exits(i);
  lastcell=info.grid.last.cell(info.grid.last.id==id);
  if info.multichannel
    channel=mod(i-1,6)+1;
  else
    channel=1;
  end
  lastpitch=info.grid.pitch(lastcell);
  oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(lastpitch),int32(channel)});
  oscmsgout('TO','/pf/grid/note',{int32(id),int32(-1)});
  info.grid.last.cell=info.grid.last.cell(info.grid.last.id~=id);
  info.grid.last.id=info.grid.last.id(info.grid.last.id~=id);
  if debug
    fprintf('ID %d exitted from grid %d\n', id, lastpitch);
  end
end

for i=1:length(info.entries)
  id=info.entries(i);
  ids=[info.snap.hypo.id];
  pos=info.snap.hypo(ids==id).pos;
  dist=sqrt((pos(1)-info.grid.position(:,1)).^2 + (pos(2)-info.grid.position(:,2)).^2);
  [newdist,newcell]=min(dist);
  if info.multichannel
    channel=mod(i-1,6)+1;
  else
    channel=1;
  end
  newpitch=info.grid.pitch(newcell);
  oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
  oscmsgout('TO','/pf/grid/note',{int32(id),int32(newpitch)});
  info.grid.last.cell(end+1)=newcell;
  info.grid.last.id(end+1)=id;
  if debug
    fprintf('ID %d entered to grid %d\n', id, newpitch);
  end
end
    
