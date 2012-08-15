% oscupdate - process incoming OSC messages and send outgoing OSC updates
% Update OSC destinations with a new set of locations for the targets
function flags=oscupdate(p,sampnum,snap,prevsnap)
  global oscsetup
  global info

  flags=struct('needcal',false,'quit',false);
  debug=true;
  log=true;
  
  % Setup info structure to pass things into apps
  if isempty(info) || nargin<4
    info=struct('duration',120,'velocity',127,'multichannel',1,'volume',1,'tempo',120,'scale',1,'key',1,'pgm',ones(1,6),'refresh',true,'channels',zeros(1,16),'preset',0);
    % Channel(k) is the id that is on channel k, or 0 if not in use
    [s,us]=getscales();
    info.scales={us.name};
    info.keys={'C','C#','D','D#','E','F','F#','G','G#','A','A#','B'};
    info.pgms=gmpgmnames();
  end

  % Table of apps
  if ~isfield(info,'apps')
    info.apps=struct('name',{'Guitar',   'CircSeq','HotSpots',   'Grid'},...
                     'fn'  ,{@app_guitar,@app_cseq,@app_hotspots,@app_grid},...
                     'pos' ,{'5/1',      '5/2',     '5/3',       '5/4'});
    for i=1:length(info.apps)
      info.apps(i).index=i;
    end
  end

  if ~isfield(info,'currapp')
    info.currapp=info.apps(end);
  end

  % Table of sample sets
  if ~isfield(info,'samples')
    info.samples=struct('name',{'MIDI',   'A','B',   'C'},...
                        'pos' ,{'5/1',      '5/2',     '5/3',       '5/4'});
    for i=1:length(info.samples)
      info.samples(i).index=i;
    end
  end

  if ~isfield(info,'currsample')
    info.currsample=info.samples(1);
  end

  % Table of presets (set of program numbers for each channel)
  % Can be shorter than pgms[], only sets first N
  if ~isfield(info,'presets')
    info.presets={[1 1 1 1 1 1],25:2:35,[53,54,55,86,92],41:46};
    info.presetnames={'Piano','Guitar','Voices','Strings'};
  end
  
  info.p=p;
  info.sampnum=sampnum;
  if nargin>=3
    info.snap=snap;
  else
    info.snap=[];
  end
  if nargin>=4
    info.prevsnap=prevsnap;
  else
    info.prevsnap=[];
  end
  
  if isempty(oscsetup)
    % Need to init OSC
    oscinit;
  end

  % Setup client messages
  m={};
  % Just began
  running=true;
  info.juststarted=false;
  if sampnum==1
    info=info.currapp.fn(info,'start');
    oscmsgout([],'/pf/started',{});
    oscsetup.starttime=snap.when;
    info.juststarted=true;
  elseif nargin<3
    info=info.currapp.fn(info,'stop');
    oscmsgout([],'/pf/stopped',{});
    running=false;
    stopping=true;
  end

  if info.juststarted
    info.refresh=true;   % First call - do a dump
  end

  % Check for incoming messages to server
  while true
    % Non-blocking receive
    msgin=osc_recv(oscsetup.server,0.0);
    if isempty(msgin)
      % No message 
      break;
    end
    for i=1:length(msgin)
      rcvdmsg=msgin{i};
      if log
        osclog('msg',rcvdmsg,'server',oscsetup.server);
      end
      if debug
        fprintf('%s->%s\n', rcvdmsg.src, formatmsg(rcvdmsg.path,rcvdmsg.data));
      end
      
      % Messages to relay
      handled=false;
      if strncmp(rcvdmsg.path,'/led/',5)
        % Relay message to LED Server
        oscmsgout('LD',rcvdmsg.path,rcvdmsg.data);
        handled=true;
      end
      
      if strncmp(rcvdmsg.path,'/midi/',6) || strcmp(rcvdmsg.path,'/tempo')
        % Relay message to MAX
        oscmsgout('MAX',rcvdmsg.path,rcvdmsg.data);
        handled=true;
      end

      % Messages to process
      if strncmp(rcvdmsg.path,'/pf/dest/add/port',17)
        [host,port,proto]=spliturl(rcvdmsg.src);
        url=sprintf('%s://%s:%d',proto,host,rcvdmsg.data{1});
        [tdhost,tdport]=getsubsysaddr('TO');
        if length(rcvdmsg.data)>1
          % Add with Ident
          ident=rcvdmsg.data{2};
        elseif length(rcvdmsg.path)>=19
          ident=rcvdmsg.path(19:end);
        else
          fprintf('Received %s without an ident field\n', rcvdmsg.path);
          ident='';
        end
        oscadddest(url,ident);
        % Would be good to forward this message to LEDServer but need to make it look like its src address was rcvdmsg.src
        info.refresh=true;   % Always start with a dump
      elseif strcmp(rcvdmsg.path,'/pf/dest/remove/port')
        [host,port,proto]=spliturl(rcvdmsg.src);
        url=sprintf('%s://%s:%d',proto,host,rcvdmsg.data{1});
        oscrmdest(url);
      elseif strcmp(rcvdmsg.path,'/pf/dump')
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/pf/calibrate')
        if length(rcvdmsg.data)<1 || rcvdmsg.data{1}>0.5
          % Don't respond to button up event (with data=0.0), only button down (1.0)
          flags.needcal=true;
          fprintf('Request for calibration received from %s\n',rcvdmsg.src);
        end
      elseif strcmp(rcvdmsg.path,'/sound/setapp')
        found=false;
        for i=1:length(info.apps)
          if strcmp(rcvdmsg.data{1},info.apps(i).name)
            info=info.currapp.fn(info,'stop');
            info.currapp=info.apps(i);
            found=true;
            fprintf('Setting PF to app %s\n', info.currapp.name);
            info=info.currapp.fn(info,'start');
          end
        end
        if ~found
          fprintf('Unknown app: %s %s\n', rcvdmsg.path,rcvdmsg.data{1});
        end
        info.refresh=true;
      elseif strncmp(rcvdmsg.path,'/sound/app/buttons/',17)
        % TouchOSC multibutton controller
        fprintf('Got message: %s\n', formatmsg(rcvdmsg.path,rcvdmsg.data));
        if rcvdmsg.data{1}==1
          pos=rcvdmsg.path(20:end);
          appnum=find(strcmp(pos,{info.apps.pos}));
          if ~isempty(appnum)
            % Turn off old block
            info=info.currapp.fn(info,'stop');
            info.currapp=info.apps(appnum);
            fprintf('Switching to app %d: %s\n',appnum,info.currapp.name);
            info=info.currapp.fn(info,'start');
            info.refresh=true;
          else
            fprintf('Attempt to switch to unsupported app %s\n', pos);
          end
        end
      elseif strncmp(rcvdmsg.path,'/sample/app/buttons/',17)
        % TouchOSC multibutton controller for sample set
        fprintf('Got message: %s\n', formatmsg(rcvdmsg.path,rcvdmsg.data));
        if rcvdmsg.data{1}==1
          pos=rcvdmsg.path(21:end);
          samplenum=find(strcmp(pos,{info.samples.pos}));
          if ~isempty(samplenum)
            info.currsample=info.samples(samplenum);
            fprintf('Switching to sample %d: %s\n',samplenum,info.currsample.name);
            info.refresh=true;
          else
            fprintf('Attempt to switch to unsupported sample set %s\n', pos);
          end
        end
      elseif strncmp(rcvdmsg.path,'/midi/preset',12)  && rcvdmsg.data{1}==1
        newpreset=16-str2num(rcvdmsg.path(14:end));   % Numbered from bottom 1..15
        if newpreset>=1 && newpreset<=length(info.presets)
          info.preset=newpreset;
          ps=info.presets{info.preset};
          for i=1:length(ps)
            info.pgm(i)=ps(i);
          end
          fprintf('Set preset to %d-%s (msg=%s)\n',newpreset, info.presetnames{newpreset}, rcvdmsg.path);
          info.refresh=true;
        else
          fprintf('Invalid preset %d (msg=%s)\n',newpreset, rcvdmsg.path);
        end
      elseif strcmp(rcvdmsg.path,'/touchosc/seq/scale/incr') && rcvdmsg.data{1}==1
        info.scale=mod(info.scale,length(info.scales))+1;
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/touchosc/seq/scale/decr') && rcvdmsg.data{1}==1
        info.scale=mod(info.scale-2,length(info.scales))+1;
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/touchosc/seq/key/incr') && rcvdmsg.data{1}==1
        info.key=mod(info.key,length(info.keys))+1;
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/touchosc/seq/key/decr') && rcvdmsg.data{1}==1
        info.key=mod(info.key-2,length(info.keys))+1;
        info.refresh=true;
      elseif strncmp(rcvdmsg.path,'/midi/pgm/',10)
        slashes=find(rcvdmsg.path=='/');
        index=str2num(rcvdmsg.path(slashes(3)+1:end));
        %fprintf('info.pgm(%d)=%d\n', index, rcvdmsg.data{1});
        if index>=1 && index<=length(info.pgm)
          if rcvdmsg.data{1}<0 && info.pgm(index)>1
            info.pgm(index)=info.pgm(index)-1;
          elseif rcvdmsg.data{1}>0 && info.pgm(index)<length(info.pgms)
            info.pgm(index)=info.pgm(index)+1;
          end
          info.preset=0;
          info.refresh=true;
        end
      elseif strcmp(rcvdmsg.path,'/midi/velocity')
        info.velocity=round(rcvdmsg.data{1});
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/midi/duration')
        info.duration=round(rcvdmsg.data{1});  % Duration in ticks
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/midi/multichannel')
        info.multichannel=rcvdmsg.data{1};  % Duration in ticks
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/tempo')
        info.tempo=round(rcvdmsg.data{1});  % Speed in BPM
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/volume')
        info.volume=rcvdmsg.data{1};  % Volume 0-1.0
        info.refresh=true;
      elseif strcmp(rcvdmsg.path,'/max/seqt/pos')
        if strcmp(info.currapp.name,'CircSeq') && running
          oscmsgout('LD',rcvdmsg.path,rcvdmsg.data);
        end
      elseif strcmp(rcvdmsg.path,'/ping')
        % ignore
      elseif ~handled
        fprintf('Unknown OSC message: %s\n', rcvdmsg.path);
      end
    end
  end

  if info.refresh
    fprintf('Sending refresh to OSC listeners\n');
    active=single(p.layout.active);
    oscmsgout([],'/pf/set/minx',{min(active(:,1))});
    oscmsgout([],'/pf/set/maxx',{max(active(:,1))});
    oscmsgout([],'/pf/set/miny',{min(active(:,2))});
    oscmsgout([],'/pf/set/maxy',{max(active(:,2))});

    % Refresh UI
    oscmsgout('TO','/sound/app/name',{info.currapp.name});
    for i=1:length(info.apps)
      oscmsgout('TO',sprintf('/sound/app/buttons/%s',info.apps(i).pos),{i==info.currapp.index});
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
    if nargin>=3
      oscmsgout([],'/pf/set/npeople',{int32(length(snap.hypo))});
    end

    for channel=1:length(info.channels)
      id=info.channels(channel);
      if id>0
        oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{1});
        oscmsgout('TO',sprintf('/touchosc/loc/%d/color',channel),{col2touchosc(id2color(id,p.colors))});
        oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{num2str(id)});
      else
        oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{0});
        oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{''});
      end
    end

    for i=1:length(info.pgm)
      oscmsgout('TO',sprintf('/midi/pgm/%d/value',i),{info.pgms{info.pgm(i)}});
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
    oscmsgout('TO','/tempo',{info.tempo});
    oscmsgout('MAX','/pf/pass/transport',{'tempo',info.tempo});
    oscmsgout('TO','/tempo/value',{info.tempo});
    oscmsgout('TO','/volume',{info.volume});
    oscmsgout('TO','/midi/multichannel',{int32(info.multichannel)});

    oscmsgout('TO','/touchosc/seq/scale/value',{[num2str(info.scale),': ',info.scales{info.scale}]});
    oscmsgout('TO','/touchosc/seq/key/value',{info.keys{info.key}});

  end

  if running
    elapsed=(snap.when-oscsetup.starttime)*24*3600;
    
    ids=[snap.hypo.id];
    if nargin>=4
      previds=[prevsnap.hypo.id];
    else
      previds=[];
    end
    
    % Compute entries, exits
    info.entries=setdiff(ids,previds);
    info.exits=setdiff(previds,ids);
    info.updates=intersect(ids,previds);
    
    if mod(sampnum,20)==0
      oscmsgout([],'/pf/frame',{int32(sampnum) },'debug',false);
    end
    
    for i=info.entries
      oscmsgout([],'/pf/entry',{int32(sampnum),elapsed,int32(i)});
      channel=find(info.channels==0,1);
      if isempty(channel)
        fprintf('No more channels available to allocate to ID %d\n', i);
      else
        oscmsgout('TO',sprintf('/touchosc/loc/%d/color',channel),{col2touchosc(id2color(i,p.colors))});
        oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{1});
        oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{num2str(i)});
        info.channels(channel)=i;
      end
    end

    people=length(ids);
    prevpeople=length(previds);
    if people~=prevpeople
      oscmsgout([],'/pf/set/npeople',{int32(people)});
    end
    for i=1:length(snap.hypo)
      h=snap.hypo(i);
      % Compute groupid -- lowest id in group, or 0 if not in a group
      sametnum=find(h.tnum==[snap.hypo.tnum]);
      groupid=snap.hypo(min(sametnum)).id;
      % TODO - should send to all destinations except TO
      oscmsgout('LD','/pf/update',{int32(sampnum), elapsed,int32(h.id),h.pos(1),h.pos(2),h.velocity(1),h.velocity(2),h.majoraxislength,h.minoraxislength,int32(groupid),int32(length(sametnum))});
      xypos=h.pos ./max(abs(p.layout.active));
      channel=id2channel(info,h.id);
      oscmsgout('TO',sprintf('/touchosc/loc/%d',channel),{xypos(2),xypos(1)});
    end
  end

  if running
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
  end

  if running
    info=info.currapp.fn(info,'update');
  end
  
  
  % Need to handle exits last since app.fn may have needed to know which channel the id was on
  if running
    for i=info.exits
      oscmsgout([],'/pf/exit',{int32(sampnum),elapsed,int32(i)});
      channel=id2channel(info,i);
      %        oscmsgout('TO',sprintf('/touchosc/loc/%d/color',channel),{'red'});
      oscmsgout('TO',sprintf('/touchosc/loc/%d/visible',channel),{0});
      oscmsgout('TO',sprintf('/touchosc/id/%d',channel),{''});
      info.channels(channel)=0;
    end
  end

  info.refresh=false;
end

function m=msg(path,data)
  if iscell(data)
    m=struct('path',{path},'data',{data});
  else
    m=struct('path',{path},'data',{{data}});
  end
end
