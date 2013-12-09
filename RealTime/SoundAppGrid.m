% Grid app
classdef SoundAppGrid < SoundApp
  properties
    minx;
    miny;
    maxx;
    maxy;
    nx=8;
    ny=9;
    hysteresis;
    maxcliptime=0;
    position=[];
    pitch=[];
    actives=struct('cell',{},'id',{},'triggertime',{},'offtime',{});
    lastgroupfired=0;   % Last time a group clip was fired
    lastgrouptrack=0;
  end
  
  methods
    function obj=SoundAppGrid(uiposition)
      obj=obj@SoundApp(uiposition);
      obj.name='Grid';
    end
    
    function info=start(obj,p,info)
      info.max=0;
      info.ableton=1;
      obj.minx=min(p.layout.active(:,1))+0.1;
      obj.maxx=max(p.layout.active(:,1))-0.5;
      obj.miny=min(p.layout.active(:,2))+0.1;
      obj.maxy=max(p.layout.active(:,2))-0.1;
      dx=(obj.maxx-obj.minx)/obj.nx;
      dy=(obj.maxy-obj.miny)/obj.ny;
      obj.hysteresis=min([dx,dy])/10;
      ind=1;
      for i=obj.nx:-1:1
        if i==1 || i==obj.nx
          inset=2;
        elseif i==2 || i==obj.nx-1
          inset=1;
        else
          inset=0;
        end
        for j=1+inset:obj.ny-inset
          pos=[obj.minx+(i-0.5)*dx,obj.miny+(j-0.5)*dy];
          if inpolygon(pos(1),pos(2),p.layout.active(:,1),p.layout.active(:,2))
            obj.position(end+1,:)=pos;
          end
          ind=ind+1;
        end
      end
      if (size(obj.position,1)~=info.al.numclips())
        fprintf('Warning: Grid has %d locations, but only %d scenes in Ableton -- wrapping extras\n', size(obj.position,1), info.al.numclips());
      end
      cell=1:size(obj.position,1);
      obj.pitch=cell+round(64-mean(cell));
      if ~isempty(info.song)
        info.cm.setnumchannels(info.al.numsongtracks(info.song));
        info.al.settempo(info.al.getsongtempo(info.song));   % Set song tempo
      end
      obj.updategriddisplay(p,info);
    end

    function stop(obj,p,info)
    % Turn off everything
      info.al.stop();
      info.al.stopalltracks();
      obj.actives=obj.actives([]);   % Turn off all active cells
      obj.updategriddisplay(p,info);
    end

    function plot(obj,p,info)
      setfig('grid');
      clf;
      plotlayout(p.layout,0);
      hold on;
      dx=(obj.maxx-obj.minx)/obj.nx;
      dy=(obj.maxy-obj.miny)/obj.ny;
      for i=0:obj.nx
        plot((obj.minx+i*dx)*[1 1],[obj.miny,obj.maxy],'g');
        if i>0
          plot((obj.minx+i*dx)*[1 1]-obj.hysteresis,[obj.miny,obj.maxy],'c:');
        end
        if i<obj.nx
          plot((obj.minx+i*dx)*[1 1]+obj.hysteresis,[obj.miny,obj.maxy],'c:');
        end
      end
      for j=0:obj.ny
        plot([obj.minx,obj.maxx],(obj.miny+j*dy)*[1 1],'g');
        if j>0
          plot([obj.minx,obj.maxx],(obj.miny+j*dy)*[1 1]-obj.hysteresis,'c:');
        end
        if j<obj.ny
          plot([obj.minx,obj.maxx],(obj.miny+j*dy)*[1 1]+obj.hysteresis,'c:');
        end
      end
      for i=1:size(obj.position,1)
        text(obj.position(i,1)-dx/4,obj.position(i,2),sprintf('%d',i));
      end
      title(sprintf('Grid layout (%d wide by %d high with %d cells) (%.2fm x %.2fm)',obj.nx, obj.ny, size(obj.position,1),dx,dy));
      pause(0.1);
    end

    function info=update(obj,p,info)
      prioractive=obj.actives;   % Save prior active state

      % Check for updates
      for i=1:length(info.updates)
        id=info.updates(i);
        ids=[info.snap.hypo.id];
        pos=info.snap.hypo(ids==id).pos;
        dist=sqrt((pos(1)-obj.position(:,1)).^2 + (pos(2)-obj.position(:,2)).^2);
        [newdist,newcell]=min(dist);
        active=obj.actives([obj.actives.id]==id);
        if isempty(active)       % Since this is an update, there was already an entry
                                 % This could occur if we switched to SoundAppGrid in midstream so there were already entries not processed here
          fprintf('SoundAppGrid: len(active.cell)~=1, active.cell=%s\n', sprintf('%d ', active.cell));
          obj.actives=[obj.actives,struct('cell',newcell,'id',id,'triggertime',now,'offtime',now)];
          % No notes yet, only if they move to a new grid
          continue;
        end
        if length(active)==1 && active.cell==newcell
          % No change
          continue;
        end
        % Check if we've moved cleanly to another grid (to avoid oscillating near boundary)
        activedist=norm(obj.position(active.cell,:)-pos);
        if (activedist-newdist)/2 < obj.hysteresis
          % Not far enough
          fprintf('Sticking at %d (d=%.2f) instead of %d (%.2f)\n',  active.cell, activedist, newcell, newdist);
          newcell=active.cell;
        end
        if newcell~=active.cell
          obj.actives([obj.actives.id]==id)=struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan);
          channel=info.cm.id2channel(id);
          activepitch=obj.pitch(active.cell);
          newpitch=obj.pitch(newcell);
          if info.max
            oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(activepitch),int32(channel)});
            oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
          end
          if info.ableton && ~isempty(info.song)
            %pan=pos(2)/max(abs(p.layout.active(:,2)));
            info.al.cliptrigger(info.song,channel,mod(newcell-1,info.al.numclips())+1);
          end
          if obj.debug
            fprintf('ID %d (channel %d) moved from grid %d (%.2f) to grid %d (%.2f)\n', id, channel, active.cell, activedist, newcell, newdist);
          end
        end
      end

      for i=1:length(info.exits)
        id=info.exits(i);
        active=obj.actives([obj.actives.id]==id);
        activepitch=obj.pitch(active.cell);
        channel=info.cm.id2channel(id);
        if info.max
          oscmsgout('MAX','/pf/pass/noteoff',{int32(id),int32(activepitch),int32(channel)});
        end
        if info.ableton && ~isempty(info.song)
          info.al.stopsongtrack(info.song,channel);
        end
        obj.actives=obj.actives([obj.actives.id]~=id);
        if obj.debug
          fprintf('ID %d exitted from grid %d\n', id, active.cell-1);
        end
      end

      for i=1:length(info.entries)
        id=info.entries(i);
        if any([obj.actives.id]==id)
          fprintf('Already have id %d inside\n', id);
          continue;
        end
        ids=[info.snap.hypo.id];
        pos=info.snap.hypo(ids==id).pos;
        dist=sqrt((pos(1)-obj.position(:,1)).^2 + (pos(2)-obj.position(:,2)).^2);
        [~,newcell]=min(dist);
        channel=info.cm.id2channel(id);
        newpitch=obj.pitch(newcell);
        if info.max
          oscmsgout('MAX','/pf/pass/noteon',{int32(id),int32(newpitch),int32(info.velocity),int32(info.duration),int32(channel)});
        end
        if info.ableton && ~isempty(info.song)
          %pan=pos(2)/max(abs(p.layout.active(:,2)));
          info.al.cliptrigger(info.song,channel,mod(newcell-1,info.al.numclips())+1);
        end
        obj.actives=[obj.actives,struct('cell',newcell,'id',id,'triggertime',now,'offtime',nan)];
        if obj.debug
          fprintf('ID %d entered at grid %d\n', id, newcell);
        end
      end

      if info.ableton
        for i=1:length(info.groupsformed)
          gid=info.groupsformed(i);
          idset=info.groupmap.gid2idset(gid);
          gtracknum=length(idset)-1;
          gtrack=[];
          while gtracknum>=1
            gtrackname=sprintf('MV%d',length(idset)-1);
            gtrack=info.al.findtrack(gtrackname);
            if ~isempty(gtrack)
              break;
            end
            fprintf('%s not found, trying prior track\n', gtrackname);
            gtracknum=gtracknum-1;
          end
          if ~isempty(gtrack)
            if ~info.al.isplaying(gtrack) % Don't interfere with clip that is playing
              fprintf('Triggering clip (%d,%d) due to group %d formation\n', gtrack,gid, gid);
              info.al.playclip(gtrack,gid);
              obj.lastgroupfired=now;
              obj.lastgrouptrack=gtrack;
            else
              fprintf('Not triggering group track %d since something is already playing\n');
            end
          else
            fprintf('Group clip track %s not found in AL\n', gtrackname);
          end
        end

        for i=1:length(info.groupsbroken)
          % Nothing to do
        end
      end
          
      if ~isempty(info.exits) && isempty(info.entries) && isempty(info.updates)
        % Active person exitted
        fprintf('Last person exitted grid\n');
        if info.ableton
          info.al.stop();
          info.al.stopalltracks();
        end
        info.song=mod(info.song,info.al.numsongs())+1;
      end

      if ~isempty(info.exits) || ~isempty(info.entries) || ~isempty(info.updates)
        obj.updategriddisplay(p,info, prioractive);
      end

      if info.ableton && ~isempty(info.song) && obj.maxcliptime>0  % Disabled
                            % Terminate clips after a certain time
        for i=1:length(obj.actives)
          l=obj.actives(i);
          if isnan(l.offtime) && (now-l.triggertime)*24*3600>obj.maxcliptime
            channel=info.cm.id2channel(l.id);
            info.al.stopsongtrack(info.song,channel);
            obj.actives(i).offtime=now;
            if obj.debug
              fprintf('Ended clip on channel %d\n', channel);
            end
          end
        end
      end
    end


    function updategriddisplay(obj,p,info, prioractive)
    % Update grid display
      if nargin>3
        scenes=union([obj.actives.cell],[prioractive.cell]);
      else
        scenes=1:info.al.numscenes;
        fprintf('Setting %d scenes\n',info.al.numscenes);
      end
      for i=scenes
        ids=[obj.actives([obj.actives.cell]==i).id];
        if isempty(ids)
          %      oscmsgout({'TO','VD'},sprintf('/grid/cell/%d',i),{sprintf('%d',i)});
          oscmsgout({'TO','VD'},sprintf('/grid/cell/%d',i),{''});
          oscmsgout({'TO','VD'},sprintf('/grid/cell/%d/color',i),{'gray'});
        else
          txt='';
          col='gray';
          for j=1:length(ids)
            channel=info.cm.id2channel(ids(j));
            if ~isempty(info.song) && info.al.haveclip(info.song,channel,mod(i-1,info.al.numclips())+1)
              txt=[txt,sprintf('T%d ',channel)];
              col=col2touchosc(id2color(ids(j),p.colors));
            else
              txt=[txt,sprintf('Stop%d ',channel)];
            end
          end
          oscmsgout({'TO','VD'},sprintf('/grid/cell/%d',i),{txt});
          oscmsgout({'TO','VD'},sprintf('/grid/cell/%d/color',i),{col});
        end
        if nargin==2
          pause(0.01);   % Don't overrun when setting up the entire grid
        end
      end
      if ~isempty(info.song)
        oscmsgout({'TO','VD'},'/grid/song',{info.al.getsongname(info.song)});
      else
        oscmsgout({'TO','VD'},'/grid/song',{''});
      end
    end
    
  end
end
