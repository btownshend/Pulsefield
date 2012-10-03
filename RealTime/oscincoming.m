% oscincoming - process incoming OSC messages
function info=oscincoming(p,info)
  debug=false;
  
  % Check for incoming messages to server
  while true
    % Non-blocking receive
    rcvdmsg=oscmsgin('MPO',0.0);
    if isempty(rcvdmsg)
      break;
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
    
    if strcmp(rcvdmsg.path,'/snapshot/trigger')
      oscmsgout('SN',rcvdmsg.path,rcvdmsg.data);
      handled=true;
    end

    if strncmp(rcvdmsg.path,'/midi/',6) || strcmp(rcvdmsg.path,'/tempo')
      % Relay message to MAX
      oscmsgout('MAX',rcvdmsg.path,rcvdmsg.data);
      handled=true;
    end

    % Messages to process
    if strncmp(rcvdmsg.path,'/pf/dest/add/port',17)
      [host,~,proto]=spliturl(rcvdmsg.src);
      url=sprintf('%s://%s:%d',proto,host,rcvdmsg.data{1});
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
      [host,~,proto]=spliturl(rcvdmsg.src);
      url=sprintf('%s://%s:%d',proto,host,rcvdmsg.data{1});
      oscrmdest(url);
    elseif strcmp(rcvdmsg.path,'/pf/dump')
      info.refresh=true;
    elseif strcmp(rcvdmsg.path,'/pf/calibrate')
      if length(rcvdmsg.data)<1 || rcvdmsg.data{1}>0.5
        % Don't respond to button up event (with data=0.0), only button down (1.0)
        info.needcal=1;
        fprintf('Request for calibration received from %s\n',rcvdmsg.src);
      end
    elseif strcmp(rcvdmsg.path,'/pf/reset')
      if length(rcvdmsg.data)<1 || rcvdmsg.data{1}>0.5
        % Don't respond to button up event (with data=0.0), only button down (1.0)
        info.needcal=2;
        fprintf('Request for reset received from %s\n',rcvdmsg.src);
      end
    elseif strcmp(rcvdmsg.path,'/sound/setapp')
      found=false;
      for i=1:length(info.apps)
        if strcmp(rcvdmsg.data{1},info.apps{i}.name)
          info.currapp.stop(p,info);
          info.currapp=info.apps{i};
          found=true;
          fprintf('Setting PF to app %s\n', info.currapp.getname());
          info=info.currapp.start(p,info);
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
        appnum=[];
        for j=1:length(info.apps)
          if strcmp(pos,info.apps{j}.uiposition)
            appnum=j;
            break;
          end
        end

        if ~isempty(appnum)
          % Turn off old block
          info.currapp.stop(p,info);
          info.currapp=info.apps{appnum};
          fprintf('Switching to app %d: %s\n',appnum,info.currapp.getname());
          info=info.currapp.start(p,info);
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
      slashes=strfind(rcvdmsg.path,'/');
      newpreset=16-str2double(rcvdmsg.path(slashes(3)+1:slashes(4)-1));   % Numbered from bottom 1..15
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
    elseif strcmp(rcvdmsg.path,'/touchosc/seq/scale/incr')
      if rcvdmsg.data{1}==1
        info.scale=mod(info.scale,length(info.scales))+1;
        info.refresh=true;
      end
    elseif strcmp(rcvdmsg.path,'/touchosc/seq/scale/decr')
      if rcvdmsg.data{1}==1
        info.scale=mod(info.scale-2,length(info.scales))+1;
        info.refresh=true;
      end
    elseif strcmp(rcvdmsg.path,'/touchosc/seq/key/incr')
      if rcvdmsg.data{1}==1
        info.key=mod(info.key,length(info.keys))+1;
        info.refresh=true;
      end
    elseif strcmp(rcvdmsg.path,'/touchosc/seq/key/decr')
      if rcvdmsg.data{1}==1
        info.key=mod(info.key-2,length(info.keys))+1;
        info.refresh=true;
      end
    elseif strcmp(rcvdmsg.path,'/touchosc/song/incr')
      if rcvdmsg.data{1}==1
        info.song=mod(info.song,info.al.numsongs())+1;
        info.al.stopalltracks();
        info.al.settempo(info.al.getsongtempo(info.song));
        fprintf('Switched to song %d\n', info.song);
      end
    elseif strncmp(rcvdmsg.path,'/touchosc/seq/',14)
      if rcvdmsg.data{1}==1
        % Use this as a signal to clear that square
        slashes=find(rcvdmsg.path=='/');
        sx=str2num(rcvdmsg.path(slashes(3)+1:slashes(4)-1));
        sy=str2num(rcvdmsg.path(slashes(4)+1:end));
        fprintf('Clearing %s (%d,%d)\n', rcvdmsg.path(1:13),sx,sy);
        oscmsgout('TO',rcvdmsg.path(1:13),{int32(sx),int32(sy),int32(0)});
      end
    elseif strcmp(rcvdmsg.path,'/touchosc/ableton/reload')
      % Reload track/clip data from AL -- may take a while
      oscmsgout('TO','/touchosc/ableton/reload/color',{'red'});
      info.al.update();
      oscmsgout('TO','/touchosc/ableton/reload/color',{'green'});
    elseif strncmp(rcvdmsg.path,'/touchosc/newtrack/',19)
      slashes=find(rcvdmsg.path=='/');
      channel=str2double(rcvdmsg.path(slashes(3)+1:end));
      % Reassign ID to another channel
      id=info.cm.channel2id(channel);  
      if ~isempty(id)
        info.cm.deleteid(id);
        newchannel=info.cm.newchannel(id);
        fprintf('Reassigning ID %d on channel %d to a channel %d\n', id, channel,newchannel);
        if info.ableton
          info.al.stopalltracks();
        end
      end
    elseif strcmp(rcvdmsg.path,'/max/transport')
      % Received bars,beats,ticks,res,tempo,time sig1, time sig2
      info.maxtransport=rcvdmsg.data;
      % fprintf('Max transport: %s (%.2f)\n', formatmsg('',rcvdmsg.data),relpos);
      if info.max
        % Pass update to LED server
        oscmsgout('LD',rcvdmsg.path,rcvdmsg.data);
        if strcmp(info.currapp.getname(),'CSeq')
          % Update pointer in TouchOSC display
          relpos=(info.maxtransport{2}-1+info.maxtransport{3}/480)/info.maxtransport{6};
          oscmsgout('TO','/touchosc/seq/pos',{relpos*15+0.5/16});
        end
      end
    elseif strncmp(rcvdmsg.path,'/midi/pgm/',10)
      slashes=find(rcvdmsg.path=='/');
      index=str2double(rcvdmsg.path(slashes(3)+1:end));
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
    elseif strcmp(rcvdmsg.path,'/enable/ableton')
      info.ableton=rcvdmsg.data{1};
      fprintf('Ableton Enable = %d\n',info.max);
      if ~info.ableton
        info.al.stop();
      end
      info.refresh=true;
    elseif strcmp(rcvdmsg.path,'/enable/max')
      info.max=rcvdmsg.data{1};
      fprintf('MAX Enable = %d\n',info.max);
      info.refresh=true;
    elseif strcmp(rcvdmsg.path,'/tempo')
      info.tempo=round(rcvdmsg.data{1});  % Speed in BPM
      info.al.settempo(info.tempo);
      info.refresh=true;
    elseif strcmp(rcvdmsg.path,'/volume')
      % Volume 0-1.0
      vol=rcvdmsg.data{1};
      info.volume=vol;
      if info.ableton
        fprintf('Setting AL volume to %f\n', vol);
        info.al.setvolume(vol);
      end
      info.refresh=true;
    elseif strncmp(rcvdmsg.path,'/page/',6)
      info.health.gotmsg('TO');
      info.touchpage=rcvdmsg.path(7:end);
    elseif strcmp(rcvdmsg.path,'/ping')
      % ignore
    elseif strcmp(rcvdmsg.path,'/ack')
      if rcvdmsg.data{1}==1
        % Should actually come in on MPL port
        info.health.gotmsg('LD');
      elseif rcvdmsg.data{1}==2
        info.health.gotmsg('FE');
      elseif rcvdmsg.data{1}==3
        info.health.gotmsg('MX');
      end
      % ignore
    elseif ~handled
      fprintf('Unknown OSC message: %s\n', rcvdmsg.path);
    end
  end

  % Check for incoming messages to server from LedServer
  while true
    % Non-blocking receive
    rcvdmsg=oscmsgin('MPL',0.0);
    if isempty(rcvdmsg)
      break;
    end
    info.health.gotmsg('LD');
    if debug
      fprintf('%s->%s\n', rcvdmsg.src, formatmsg(rcvdmsg.path,rcvdmsg.data));
    end
  end

end
