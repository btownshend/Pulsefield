% oscupdate - send outgoing OSC updates
function info=oscupdate(p,info,sampnum,snap,prevsnap)
  debug=true;
  log=false;
  
  info.sampnum=sampnum;
  if nargin>=4
    info.snap=snap;
  else
    info.snap=[];
    snap=[];
  end
  if nargin>=5
    info.prevsnap=prevsnap;
  else
    info.prevsnap=[];
    prevsnap=[];
  end
  
  % Just began
  running=true;
  if nargin<4
    info.currapp.stop(p,info);
    oscmsgout(p.oscdests,'/pf/stopped',{});
    running=false;
  elseif info.juststarted
    info=info.currapp.start(p,info);
    oscmsgout(p.oscdests,'/pf/started',{});
    info.refresh=true;   % First call - do a dump
  end

  if info.refresh
    %fprintf('Refreshing view of Ableton Live\n');
    %info.al.update();
    info.lastvcnts(:)=-1;   % Force update of visibility counts
    
    fprintf('Sending refresh to OSC listeners\n');
    info.refresh=false;
    active=single(p.layout.active);
    oscmsgout(p.oscdests,'/pf/set/minx',{min(active(:,1))});
    oscmsgout(p.oscdests,'/pf/set/maxx',{max(active(:,1))});
    oscmsgout(p.oscdests,'/pf/set/miny',{min(active(:,2))});
    oscmsgout(p.oscdests,'/pf/set/maxy',{max(active(:,2))});

    % Refresh UI
    oscmsgout('TO','/sound/app/name',{info.currapp.getname()});
    for i=1:length(info.apps)
      oscmsgout('TO',sprintf('/sound/app/buttons/%s',info.apps{i}.uiposition),{info.apps{i}==info.currapp});
    end

    oscmsgout('TO','/sample/app/name',{info.currsample.name});
    for i=1:length(info.samples)
      oscmsgout('TO',sprintf('/sample/app/buttons/%s',info.samples(i).pos),{i==info.currsample.index});
    end
    
    if running
      col='green';
    else
      col='red';
    end
    oscmsgout('TO','/sound/app/buttons/color',{col});
    oscmsgout('TO','/sound/app/name/color',{col});
    oscmsgout('TO','/sound/app/title/color',{col});
    oscmsgout('TO','/sample/app/buttons/color',{col});
    oscmsgout('TO','/sample/app/name/color',{col});
    oscmsgout('TO','/sample/app/title/color',{col});

    % Page 2, channel displays
    if nargin>=4
      oscmsgout(p.oscdests,'/pf/set/npeople',{int32(length(snap.hypo))});
      oscmsgout('SN','/pf/set/npeople',{int32(length(snap.hypo))});
    end


    for i=1:length(info.pgm)
      oscmsgout('TO',sprintf('/midi/pgm/%d/value',i),{info.pgms{info.pgm(i)}});
      oscmsgout('VD','/midi/pgm',{int32(i), int32(info.pgm(i)), info.pgms{info.pgm(i)}});
      oscmsgout('MAX','/pf/pass/pgmout',{int32(i),int32(info.pgm(i))});
    end
    for i=1:15
      if info.preset==16-i
        oscmsgout('TO',sprintf('/midi/preset/%d/1',i),{int32(1)});
      else
        oscmsgout('TO',sprintf('/midi/preset/%d/1',i),{int32(0)});
      end
    end
    if info.preset>0
      oscmsgout('TO','/midi/presetname',{info.presetnames{info.preset}});
    else
      oscmsgout('TO','/midi/presetname',{''});
    end
    oscmsgout('TO','/midi/duration',{info.duration});
    oscmsgout('TO','/midi/duration/value',{info.duration});
    oscmsgout('TO','/midi/velocity',{info.velocity});
    oscmsgout('TO','/midi/velocity/value',{info.velocity});
    oscmsgout({'TO','VD'},'/tempo',{info.tempo});
    oscmsgout('MAX','/pf/pass/transport',{'tempo',info.tempo});
    oscmsgout('TO','/tempo/value',{info.tempo});
    fprintf('Updating TO with volume=%f\n',info.volume);
    oscmsgout('TO','/volume',{info.volume});
    oscmsgout('TO','/volume/value',{sprintf('%.1f',Ableton.slider2db(info.volume))});
    oscmsgout('TO','/enable/ableton',{int32(info.ableton)});
    oscmsgout('TO','/enable/max',{int32(info.max)});

    info.currapp.refresh(p,info);
  end

  if running
    elapsed=(snap.when-info.starttime)*24*3600;
    
    ids=[snap.hypo.id];
    if nargin>=5
      previds=[prevsnap.hypo.id];
    else
      previds=[];
    end
    
    % Compute entries, exits
    info.entries=setdiff(ids,previds);
    info.exits=setdiff(previds,ids);
    info.updates=intersect(ids,previds);

    % Compute formation, breakage of groups
    if nargin>=5
      info.groupsformed=setdiff([snap.hypo.groupid],[0,prevsnap.hypo.groupid]);
      info.groupsbroken=setdiff([prevsnap.hypo.groupid],[0,snap.hypo.groupid]);
      if ~isempty(info.groupsformed)
        fprintf('Groups formed: %s\n', sprintf('%d ',info.groupsformed));
      end
      if ~isempty(info.groupsbroken)
        fprintf('Groups broken: %s\n', sprintf('%d ',info.groupsbroken));
      end
    else
      info.groupsformed=[];
      info.groupsbroken=[];
    end
    
    if mod(sampnum,20)==0
      oscmsgout(p.oscdests,'/pf/frame',{int32(sampnum) });
    end
    
    for i=info.entries
      channel=info.cm.newchannel(i);   % Allocate a channel
      oscmsgout(p.oscdests,'/pf/entry',{int32(sampnum),elapsed,int32(i),int32(channel)});
    end

    for i=1:length(snap.hypo)
      h=snap.hypo(i);
      oscmsgout(setdiff(p.oscdests,{'TO'}),'/pf/update',{int32(sampnum), elapsed,int32(h.id),h.pos(1),h.pos(2),h.velocity(1),h.velocity(2),h.majoraxislength,h.minoraxislength,int32(h.groupid),int32(h.groupsize),int32(info.cm.id2channel(h.id))});
    end

    
    % Update LCD
    lcdsize=400;
    for i=1:length(snap.hypo)
      pos=snap.hypo(i).pos;
      lp=int32((pos+2.5)*lcdsize/5);
      lp(2)=lcdsize-lp(2);  % Inverted axis
      sz=2;
      lp=lp-sz/2;
      le=lp+sz;
      col=id2color(snap.hypo(i).id,p.colors);
      if all(col==127)
        col=[0 0 0];
      end
      col=int32(col*255);
      oscmsgout('MAX','/pf/pass/lcd',{'frgb',col(1),col(2),col(3)});
      oscmsgout('MAX','/pf/pass/lcd',{'paintoval',lp(1),lp(2),le(1),le(2)});
    end

    info=info.currapp.update(p, info);

    % Need to handle exits last since app.fn may have needed to know which channel the id was on
    for i=info.exits
      oscmsgout(p.oscdests,'/pf/exit',{int32(sampnum),elapsed,int32(i)});
      info.cm.deleteid(i);
      info.groupmap.deleteuid(i);
    end

    % Update XY display in TouchOSC
    if strcmp(info.touchpage,'positions')
      for channel=1:8
        xyids=info.cm.channel2id(channel);
        if mod(sampnum,15)==0  % Reduce number of packets -- send these messages 1/sec
          if isempty(xyids)
            oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{0});
            oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{''});
          else
            oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{1});
            idlabel='';
            for k=1:length(xyids)
              idlabel=[idlabel,sprintf('%d',xyids(k))];
              hnum=find([snap.hypo.id]==xyids(k));
              if snap.hypo(hnum).groupid~=0
                idlabel=[idlabel,sprintf('G%d',snap.hypo(hnum).groupid)];
              end
              if k<length(xyids)
                idlabel(end+1)=',';
              end
            end
            oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{idlabel});
            oscmsgout('TO',sprintf('/touchosc/loc/%d/color',channel),{col2touchosc(id2color(xyids(1),p.colors))});
          end
        end
        if ~isempty(xyids)
          % Update these every frame for active channels
          hnum=find([snap.hypo.id]==xyids(1));
          h=snap.hypo(hnum);   % Only show 1 person even though there may be multiple on same channel
          xypos=h.pos ./max(abs(p.layout.active));
          oscmsgout('TO',sprintf('/touchosc/loc/%d',channel),{xypos(2),xypos(1)});
        end
      end
    end
   
    % Update visibility count in TouchOSC
    if isfield(info,'vis')
      for c=1:size(info.vis.v,1)
        vcnts=[sum(info.vis.v(c,:)==1),sum(info.vis.v(c,:)==0),sum(isnan(info.vis.v(c,:)))];
        if ~isfield(info,'lastvcnts') || size(info.lastvcnts,1)<c
          % Initialize
          info.lastvcnts(c,:)=-1*vcnts;
        end
        if vcnts(1)~=info.lastvcnts(c,1)
          oscmsgout('TO',sprintf('/rays/visible/%d',c),{int32(vcnts(1))});
        end
        if vcnts(1)~=info.lastvcnts(c,1)
          oscmsgout('TO',sprintf('/rays/blocked/%d',c),{int32(vcnts(2))});
        end
        if vcnts(1)~=info.lastvcnts(c,1)
          oscmsgout('TO',sprintf('/rays/unknown/%d',c),{int32(vcnts(3))});
        end
        info.lastvcnts(c,:)=vcnts;
      end
    else
      fprintf('Info does not have a .vis field\n');
    end
    
    % Update number of people present after all exits, entries
    people=length(ids);
    prevpeople=length(previds);
    if people~=prevpeople
      oscmsgout(p.oscdests,'/pf/set/npeople',{int32(people)});
      oscmsgout('SN','/pf/set/npeople',{int32(people)});
    end

    
    if info.ableton
      % Read any Ableton updates
      nmsg=info.al.refresh();
      if nmsg>0
        info.health.gotmsg('AL');
      end
      info.volume=info.al.getvolume();
      info.tempo=info.al.gettempo();
    else
      % Just poll for health check,ignore result
      msg=oscmsgin('MPA',0);
      if ~isempty(msg)
        info.health.gotmsg('AL');
      end
    end

    
    if (now-info.lastping)*24*3600 > info.pinginterval
      % Ping everyone that can respond
      %fprintf('Pinging LD, MAX, AL, VD\n');
      oscmsgout('LD','/ping',{int32(1)});
      oscmsgout('MAX','/ping',{int32(3)});
      oscmsgout('AL','/live/master/volume',{});  % Force a response
      oscmsgout('VD','/ping',{int32(4)});
      % Update latency display
      oscmsgout('TO','/touchosc/latency',{round(info.latency*1000)});
      info.lastping=now;
    end
    
  end
  
  info.health.updateleds();
  
  info.juststarted=false;
end
