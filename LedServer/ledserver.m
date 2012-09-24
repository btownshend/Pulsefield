% LED server
% Receive commands via OSC, update LED's in response
function ledserver(p,doplot,simul)
  if nargin<1 || nargin>3
    fprintf('Usage: ledserver(p,doplot,simul)\n');
    return;
  end
  if nargin<2
    doplot=false;
  end
  if nargin<3
    simul=false;  % Simulate only
  end
  pfile='/Users/bst/DropBox/PeopleSensor/src/p.mat';
  if ~simul
  try
    s1=arduino_ip(1);
  catch me
    fprintf('Failed open of Arduino:  %s\n',me.message);
    fprintf('Exitting\n');
    arduino_close();
    return;
  end
  end
  
  debug=0;
  ignores={};

  if ~oscloopback('LD')
    fprintf('Loopback failed - exiting LedServer()\n');
    oscclose();
    return;
  end
  oscclose();   % Clean up any old connections

  [~,myport]=getsubsysaddr('LD');
  fprintf('Instructing frontend and Matlab processor to use port %d to send us msgs\n', myport);
  oscmsgout('FE','/vis/dest/add/port',{myport},debug);
  oscmsgout('MPL','/pf/dest/add/port',{myport,'LD'},debug);

  period=0.02;   % Update period (6 strips of LED's can update at 17.7/sec), but this also affects how much time incoming msg service gets
                 % Setting it to 0.06 gives mean rate of 10/sec, .02 gives 17/sec, 0.00 starves incoming messages
  lastupdate=0;
  apps=struct('name'  ,{'Visible',  'FollowPB',  'FollowWht',' CircSeq', 'Freeze',   'CMerge',   'MtrFollow'  },...
              'fn'    ,{@fg_visible,@fg_follow,  @fg_follow, @fg_cseq   ,@fg_freeze, @fg_cmerge, @fg_meterfollow },...
              'backfn',{@bg_white,  @bg_pulsebow,@bg_white,  @bg_blue   ,@bg_freeze, @bg_white,  @bg_white },...
              'pos'   ,{'5/1',      '5/2',       '5/3',      '5/4',      '5/5',      '4/1',      '4/2'     });
  for i=1:length(apps)
    apps(i).index=i;
  end
  currapp=apps(3);

  info=struct('state',zeros(numled(),3),'mix',zeros(numled(),3),'prevstate',[],'layout',p.layout,'vis',[],'hypo',[],'running',true,...
              'colors',{p.colors},'back',struct('maxlev',1,'minlev',0.2,'state',zeros(numled(),3)),'maxtransport',{{1,1,1,120,4,4}},...
              'maxpll',PLL,'alpll',PLL,'meter',zeros(8,2),'health',Health({'MP'}));
  latency=[];
  refresh=true;
  msgin=[];
  avglatency=0;
  refreshtime=0;
  refreshcnt=0;
  
  while true
    % Receive any OSC messages
    maxwait=max(0.0,(lastupdate-now)*24*3600+period);
    m=oscmsgin('LD',maxwait);
    if ~isempty(m)
      if ~strncmp(m.path,'/vis',4) && ~strcmp(m.path,'/pf/update') && ~strcmp(m.path,'/max/transport') && ~strcmp(m.path,'/pf/frame') && ~strcmp(m.path,'/live/songtrack/meter') && ~strcmp(m.path,'/ack') && ~strcmp(m.path,'/ping')
        fprintf('Got message: %s\n', formatmsg(m.path,m.data));
      end
      if strncmp(m.path,'/pf/dest/add/port',17)
        [host,port,proto]=spliturl(m.src);
        url=sprintf('%s://%s:%d',proto,host,m.data{1});
        if length(m.data)>1
          % Add with Ident
          ident=m.data{2};
        elseif length(m.path)>=19
          ident=m.path(19:end);
        else
          fprintf('Received %s without an ident field\n', m.path);
          ident='';
        end
        oscadddest(url,ident);
        refresh=true;   % Always start with a dump
      elseif strncmp(m.path,'/led/app/buttons/',17)
        % TouchOSC multibutton controller
        fprintf('Got message: %s\n', formatmsg(m.path,m.data));
        if m.data{1}==1
          pos=m.path(18:end);
          appnum=find(strcmp(pos,{apps.pos}));
          if ~isempty(appnum)
            % Turn off old block
            oscmsgout('TO',sprintf('/led/app/buttons/%s',currapp.pos),{0},debug);
            currapp=apps(appnum);
            fprintf('Switching to app %d: %s\n',appnum,currapp.name);
            % Turn on block for current app
            oscmsgout('TO',sprintf('/led/app/buttons/%s',currapp.pos),{1},debug);
            oscmsgout('TO','/led/app/name',{currapp.name},debug);
          else
            fprintf('Attempt to switch to unsupported app %s\n', pos);
          end
        end
      elseif strcmp(m.path,'/led/pulsebow/pperiod')
        info.pulsebow.pperiod=expcontrol(m.data{1},0.1,20);
        refresh=true;
      elseif strcmp(m.path,'/led/pulsebow/cperiod')
        info.pulsebow.cperiod=expcontrol(m.data{1},1,100);
        refresh=true;
      elseif strcmp(m.path,'/led/pulsebow/pspatial')
        info.pulsebow.pspatial=expcontrol(m.data{1},2,10000);
        refresh=true;
      elseif strcmp(m.path,'/led/pulsebow/cspatial')
        info.pulsebow.cspatial=expcontrol(m.data{1},2,10000);
        refresh=true;
      elseif strcmp(m.path,'/led/pause')
        info.running=false;
        fprintf('Stopped by %s\n', m.src);
        arduino_close();
        refresh=true;
      elseif strcmp(m.path,'/led/resume')
        info.running=true;
        fprintf('Started by %s\n', m.src);
        refresh=true;
      elseif strcmp(m.path,'/quit')
        fprintf('Received /quit command -- should exit MATLAB completely, but just closing all and returning for now\n');
        oscclose();
        arduino_close();
        return;
      elseif strcmp(m.path,'/ping')
        oscmsgout('MPL','/ack',{m.data{1}});
        info.health.gotmsg('MP');
      elseif strcmp(m.path,'/max/transport')
        % Received bars,beats,ticks,res,tempo,time sig1, time sig2
        info.maxtransport=m.data;
        info.maxpll.setref(m.data{1}*m.data{6}+m.data{2}+m.data{3}/480);
        info.maxpll.settempo(m.data{5});
        % fprintf('Max transport: %s (%.2f)\n', formatmsg('',rcvdmsg.data),relpos);
      elseif strcmp(m.path,'/vis/visible')
        % Visibility info
        c=m.data{1}+1;   % Camera
        frame=m.data{2};
        sec=m.data{3};
        usec=m.data{4};
        mincorr=m.data{5};
        blob=m.data{6};
        info.vis(c,:)=double(blob);
        info.vis(c,blob==2)=nan;
        acquired=(((sec+usec/1e6)/3600-7)/24)+datenum(1970,1,1);   % Convert to matlab datenum (assuming 7 hours offset from GMT)
        latency=(now-acquired)*24*3600;
        if latency>2/15 || mod(frame,1000)==0
          fprintf('Camera %d, frame %d latency=%.0f milliseconds (avg =%.0f ms)\n', c, frame, latency*1000,avglatency*1000);
        end
        avglatency=avglatency*0.99+latency*.01;
      elseif strcmp(m.path,'/pf/entry')
        fprintf('Entry %d\n', m.data{3});
        if isempty(info.hypo)
          ids=[];
        else
          ids=[info.hypo.id];
        end
        if ismember(m.data{3},ids)
          fprintf('Got entry for id %d, which was already inside\n',m.data{3});
        else
          info.hypo=[info.hypo,struct('id',m.data{3},'pos',[nan,nan],'velocity',[nan,nan],'entrytime',now,'channel',m.data{4})];
        end
      elseif strcmp(m.path,'/pf/exit')
        fprintf('Exit %d\n', m.data{3});
        if isempty(info.hypo)
          ids=[];
        else
          ids=[info.hypo.id];
        end
        keepind=ids~=m.data{3};
        if sum(~keepind)~=1
          fprintf('Bad exit id %d while only the following ids are inside: %s\n',m.data{3},shortlist(ids));
        end
        info.hypo=info.hypo(keepind);
      elseif strcmp(m.path,'/pf/update')
        if isempty(info.hypo)
          ids=[];
        else
          ids=[info.hypo.id];
        end
        index=find(ids==m.data{3});
        if length(index)<1
          fprintf('Missed entry of ID %d\n', m.data{3});
          info.hypo=[info.hypo,struct('id',m.data{3},'pos',[m.data{4},m.data{5}],'velocity',[m.data{6},m.data{7}],'entrytime',now,'channel',m.data{12})];
        elseif length(index)>1
          fprintf('Have multiple copies of same ids inside; ids=%s\n', shortlist(ids));
        else
          info.hypo(index).pos=[m.data{4},m.data{5}];
          info.hypo(index).velocity=[m.data{6},m.data{7}];
          info.hypo(index).channel=m.data{12};   % In case the channel was switched to TOsc
        end
      elseif strcmp(m.path,'/pf/set/npeople')
        info.npeople=m.data{1};
        if info.npeople ~= length(info.hypo)
          fprintf('Number of people (%d) not matching number of tracked hypos (%d)\n', info.npeople,length(info.hypo));
        end
        if info.npeople<length(info.hypo)
          fprintf('Discarding all hypos (will restore on update)\n');
          info.hypo=info.hypo([]);
        end
      elseif strcmp(m.path,'/seq/step')
        info.stepnum=m.data{1};
      elseif strcmp(m.path,'/live/beat')
        info.alpll.setref(m.data{1});
      elseif strcmp(m.path,'/live/tempo')
        info.alpll.settempo(m.data{1});
      elseif strcmp(m.path,'/live/songtrack/meter')
        % song (1..n), songtrack (1..n),left/right,meterlevel (0.0-1.0)
        info.meter(m.data{2},m.data{3})=m.data{4};
        %        fprintf('Got meter for channel %d, L/R %d: %.2f\n', m.data{2},m.data{3},m.data{4});
      else
        if ~ismember(m.path,ignores)
          fprintf('LEDServer: Unhandled OSC message from %s: %s - ignoring from now on.\n', m.src, m.path);
          ignores{end+1}=m.path;
        end
      end
    end
    if refresh
      % Refresh UI
      oscmsgout('TO','/led/app/name',{currapp.name},debug);
      for i=1:length(apps)
        oscmsgout('TO',sprintf('/led/app/buttons/%s',apps(i).pos),{i==currapp.index},debug);
      end

      if info.running
        col='green';
      else
        col='red';
      end
      oscmsgout('TO','/led/app/buttons/color',{col},debug);
      oscmsgout('TO','/led/app/name/color',{col},debug);
      oscmsgout('TO','/led/app/title/color',{col},debug);
      if isfield(info,'pulsebow')
        oscmsgout('TO','/led/pulsebow/cspatial/value',{sprintf('%.1f',info.pulsebow.cspatial)});
        oscmsgout('TO','/led/pulsebow/pspatial/value',{sprintf('%.1f',info.pulsebow.pspatial)});
        oscmsgout('TO','/led/pulsebow/pperiod/value',{sprintf('%.1f',info.pulsebow.pperiod)});
        oscmsgout('TO','/led/pulsebow/cperiod/value',{sprintf('%.1f',info.pulsebow.cperiod)});
      end
      refresh=false;
    end
    if info.running && (now-lastupdate)*3600*24>=period
      info.prevstate=info.state;
      info.mix=0*info.prevstate;   % Default to all background  (mix=0 -> background, mix=1 -> foreground)
      info.state=info.mix;
      info=currapp.backfn(info);   % Compute background LED colors
      info=currapp.fn(info);   % Override with foreground effect
      info=ls_updateentry(info);
      if any(size(info.prevstate)~=size(info.state))
        % First time -- initialize to something different from current state
        fprintf('Initializing prevstate\n');
        info.prevstate=info.state;
        info.prevstate(:)=255;
      end
      % Mix background and live state
      state=info.back.state.*(1-info.mix)+info.state.*info.mix;
%      state=[state;info.entrystate;info.entrystate];
      if ~simul
        ls_updateallleds(info,state);
      end
      if doplot
        ls_plotleds(info,state);
      end
      refreshtime=refreshtime+(now-lastupdate);
      refreshcnt=refreshcnt+1;
      if refreshcnt>50
        fprintf('Mean update rate %.2f/second\n', refreshcnt/(refreshtime*24*3600));
        refreshcnt=0;
        refreshtime=0;
      end
      lastupdate=now;
    end

    info.health.updateleds();
  end
end

function info=ls_updateentry(info)
  debug=1;
  if ~isfield(info,'entry')
    info.entry=struct('pperiod',4,'cperiod',20,'pspatial',65, 'cspatial',130,'nled',130,'minlev',0,'maxlev',1);
  end

  % Amplitude overall
  p0=(0:info.entry.nled-1)*2*pi/info.entry.pspatial;
  pshift=[p0;p0;p0]';
  c0=(0:info.entry.nled-1)*2*pi/info.entry.cspatial;
  cshift=[c0;c0+2*pi/3;c0+4*pi/3]';
  t=now*24*3600;
  pphase=mod(t*2*pi/info.entry.pperiod+pshift,2*pi);
  cphase=mod(t*2*pi/info.entry.cperiod+cshift,2*pi);
  amp=info.entry.minlev+(info.entry.maxlev-info.entry.minlev)*(sin(pphase)+1)/2;
  col=(sin(cphase)+1)/2;
  info.entrystate=((amp.*col).^2*.97+.03) * 127;   % Response is nonlinear (approx squared)
end

function ls_updateleds(info)
  debug=0;
  s1=arduino_ip(0);
  cmd=[];
  for i=1:size(info.state,1)
    if any(info.state(i,:)~=info.prevstate(i,:))
      % Find next LED that is a different color
      j=find(info.state(i+1:end,1)~=info.state(i,1) | info.state(i+1:end,2)~=info.state(i,2) | info.state(i+1:end,3)~=info.state(i,3),1);
      if isempty(j)
        j=size(info.state,1)-i;
      else
        j=j-1;
      end
      cmd=[cmd,setled(s1,(i:i+j)-1,info.state(i,:),0)];
      if debug
        fprintf('%s->[%d,%d,%d]; ', shortlist(i:i+j), info.state(i,:));
      end
      info.prevstate(i:i+j,:)=info.state(i:i+j,:);  % Prevent it from being done again
    end
  end
  if ~isempty(cmd)
    if debug
      fprintf('Updated LEDs using %d bytes\n',length(cmd));
    end
    cmd=[cmd,'G'];  % Show()
    awrite(s1,cmd);
  end
end

function ls_updateallleds(info,state)
  debug=0;
  s1=arduino_ip(0);
  %sync(s1);
  try
    if debug
      tic;
    end
    cmd=setallleds(s1,state,0);
    cmd=[cmd,show(s1)];
    if debug
      elapsed=toc;
      fprintf('Updated all LEDs using %d bytes in %.3f seconds\n',length(cmd),elapsed);
    end
  catch me
    fprintf('Error during setallleds: %s\n', me.message);
    arduino_close()
    % Ignore, let next update handle it
  end

end

function ls_plotleds(info,state)
  pixperled=5;
  im=127*ones(pixperled+1,size(state,1),3,'uint8');
  for i=1:pixperled
    im(i,:,:)=state;
  end
  im(pixperled+1,numled(),:)=0;
  imshow(im*2);
  pause(0.01);
end

% Update background colors of LEDs
function info=ls_pulse(info)
  debug=1;
  if ~isfield(info,'pulse')
    p0=(0:numled()-1)*2*pi/numled();
    info.pulse=struct('period',2.0, 'phaseshift',[p0;p0+2*pi/3;p0+4*pi/3]');
  end
  s1=arduino_ip(0);
  t=now*24*3600;
  phase=mod(t*2*pi/info.pulse.period+info.pulse.phaseshift,2*pi);
  amp=info.back.minlev+(info.back.maxlev-info.back.minlev)*(sin(phase)+1)/2;
  info.back.state=(amp.^2*.97+.03) * 127;   % Response is nonlinear (approx squared)
  if debug
    fprintf('t=%.1f, phase=%.0f, amp(1,:)=%.2f %.2f %.2f,lev=%.0f %.0f %.0f\n',t,phase(1,1)*180/pi,amp(1,:),lev(1,:));
  end
end

function info=bg_pulsebow(info)
  debug=1;
  if ~isfield(info,'pulsebow')
    info.pulsebow=struct('pperiod',4,'cperiod',20,'pspatial',250, 'cspatial',1000);
  end

  % Amplitude overall
  p0=(0:numled()-1)*2*pi/info.pulsebow.pspatial;
  pshift=[p0;p0;p0]';
  c0=(0:numled()-1)*2*pi/info.pulsebow.cspatial;
  cshift=[c0;c0+2*pi/3;c0+4*pi/3]';
  t=now*24*3600;
  pphase=mod(t*2*pi/info.pulsebow.pperiod+pshift,2*pi);
  cphase=mod(t*2*pi/info.pulsebow.cperiod+cshift,2*pi);
  amp=info.back.minlev+(info.back.maxlev-info.back.minlev)*(sin(pphase)+1)/2;
  col=(sin(cphase)+1)/2;
  info.back.state=((amp.*col).^2*.97+.03) * 127;   % Response is nonlinear (approx squared)
end

function info=fg_visible(info)
  info.mix(:)=0;

  if isempty(info.vis)
    fprintf('info.vis is empty!\n');
    return;
  end

  for i=1:size(info.vis,2)
    blocked=find(info.vis(:,i)==0);
    if ~isempty(blocked)
      info.state(i,:)=uint8(127*info.colors{min(blocked)+1});
      info.mix(i,:)=1;
    end
  end
end

function info=fg_meterfollow(info)
  info=fg_follow(info,1);
end
function info=fg_follow(info,usemeter)
  if nargin<2
    usemeter=0;
  end
  
  minradius=0.5;   % min radius where marker is on
  maxleds=20;    % Number of LEDs in marker when close to edge
  awidthmax=(2*pi*48/50)/sum(~info.layout.outsider) * maxleds/2;
  meanradius=(max(info.layout.active(:,2))-min(info.layout.active(:,2)))/2;
  
  for i=1:length(info.hypo)
    h=info.hypo(i);
    % Color of marker lights (track people)
    col=id2color(h.id,info.colors)*127;
    % Check for burst on entry
    dur=(now-h.entrytime)*24*3600;   % Time since entry
    if  dur < 5
      timeconst=3;  % Exponential decay of burst with this t/c
      scale=exp(-dur/timeconst);
      for j=1:3
        info.state(:,j)=info.state(:,j)+col(j);
      end
      info.mix(:)=info.mix(:)+scale;
    end

    pos=h.pos;
    % Angular width of person's marker (left and right)
    awidth(1)=awidthmax * min(1,(norm(pos)-minradius)/(meanradius-minradius));
    awidth(2)=awidth(1);

    if usemeter
      % Increase width using VU (amount above -14dB = 0.5)
      awidth=awidth + awidthmax * 10 * info.meter(h.channel,:).^2;  % Will span about 100 LEDs for 0.0-1.0 -> 2* 2.5 LED/dB at top
      fprintf('Meter=%.2f %.2f Width = %.1f %.1f LEDs\n', info.meter(h.channel,:),awidth*maxleds/awidthmax);
    end

    if norm(pos)>0.5   % At least .5m away from center
      [angle,radius]=cart2pol(pos(:,1),pos(:,2));
      langle=cart2pol(info.layout.lpos(:,1),info.layout.lpos(:,2));
      adiff=mod(angle-langle+pi,2*pi)-pi;
      % All LEDs inside active area, within awidth angle of person
      indices = find(((adiff<0 & adiff>=-awidth(1)) | (adiff>0 & adiff<=awidth(2))) & ~info.layout.outsider);   
      [~,center] = min(abs(adiff));
      % fprintf('Angle=%.1f, RFrac=%.2f, NLed=%d\n', angle*360/pi, radius/meanradius,length(indices));
      for j=1:length(indices)
        info.state(indices(j),:)=info.state(indices(j),:)+col;
        info.mix(indices(j),:)=info.mix(indices(j),:)+1;
      end
      info.state(center,:)=127;
    end
  end

  % Renormalize any with mix > 1
  mx=max(info.mix,[],2);
  fmx=find(mx>1);
  for ii=1:length(fmx)
    i=fmx(ii);
    %fprintf('Rescaling %d by %f\n', i, mx(i));
    info.mix(i,:)=info.mix(i,:)/mx(i);
  end

  % Visual feedback of how many people are inside
  % TODO - this could be in background led patterns, and also use channel map to display
  for i=1:8
    info.state(i,:)=info.colors{1}*127;
    info.mix(i,:)=1;
  end
  for i=1:length(info.hypo)
    channel=info.hypo(i).channel;
    id=info.hypo(i).id;
    info.state(channel,:)=id2color(id,info.colors)*127;
  end
end

function info=fg_cmerge(info)
  minradius=0.5;   % min radius where marker is on
  meanradius=(max(info.layout.active(:,2))-min(info.layout.active(:,2)))/2;
  
  % Randomly order so each update will sample a different person on top
  adiffs=[];
  for i=1:length(info.hypo)
    h=info.hypo(i);
    % Color of marker lights (track people)
    cols(i,:)=id2color(h.id,info.colors)*127;

    pos=h.pos;
    %fprintf('Hypo %d, radius=%f\n', i, norm(pos));
    if norm(pos)>minradius   % At least .5m away from enter
      [angle,radius]=cart2pol(pos(:,1),pos(:,2));
      langle=cart2pol(info.layout.lpos(:,1),info.layout.lpos(:,2));
      adiff=mod(langle-angle+2*pi,2*pi);
      adiff(adiff>pi)=2*pi-adiff(adiff>pi);
      adiffs(i,:)=adiff;
    end
  end
  if isempty(adiffs)
    % Nothing to show
    return;
  end

  [minangle,best]=min(adiffs,[],1);
  info.state=cols(best,:);	     % Color of closest person
  info.mix(minangle<pi/2,:)=1;      % Only on if within 90 degs of closest
  info.mix(info.layout.outsider)=0;  % Outside LEDs not affected

  % Visual feedback of how many people are inside
  % TODO - this could be in background led patterns, and also use channel map to display
  if ~isempty(info.hypo)
    ids=sort([info.hypo.id]);
  else
    ids=[];
  end

  for i=1:length(ids)
    info.state(i,:)=id2color(ids(i),info.colors)*127;
    info.mix(i,:)=1;
  end
  for i=length(ids)+1:8
    info.state(i,:)=info.colors{1}*127;
    info.mix(i,:)=1;
  end
end

function info=fg_freeze(info)
end

function info=bg_freeze(info)
end

function info=bg_white(info)
  info.back.state(:)=16;
end

function info=bg_blue(info)
  info.back.state(:,1)=0;
  info.back.state(:,2)=0;
  info.back.state(:,3)=127;
end

function info=fg_cseq(info)
  minr=0.1;  % Should be the same as in app_cseq()
  bin=zeros(16,3);
  for i=1:length(info.hypo)
    h=info.hypo(i);
    pos=h.pos;
    [angle,radius]=cart2pol(pos(:,1),pos(:,2));
    angle=mod(angle+2*pi,2*pi);
    if radius>minr
      binnum=floor(angle/(2*pi)*size(bin,1))+1;
      bin(binnum,:)=bin(binnum,:)+id2color(h.id,info.colors);
    end
  end
  % Normalize bins
  for i=1:size(bin,1)
    if any(bin(i,:)>0)
      bin(i,:)=127/max(bin(i,:))*bin(i,:);
    end
  end
  %fprintf('bins=%s\n', sprintf('[%d,%d,%d] ',bin'));
  % Set state based on bins
  langle=cart2pol(info.layout.lpos(:,1),info.layout.lpos(:,2));
  langle=mod(langle+2*pi,2*pi);
  binnum=floor(langle/(2*pi)*size(bin,1))+1;
  binnum(info.layout.outsider)=0;
  for i=1:size(bin,1)
    if any(bin(i,:)>0)
      for j=1:3
        info.state(binnum==i,j)=bin(i,j);
      end
      info.mix(binnum==i,:)=1;
    else
      info.mix(binnum==i,:)=0;
    end
  end

  % Relative position of sequencer
  relpos=mod(info.maxpll.getbeat()/info.maxtransport{6},1.0);
  if relpos<0 || relpos >1.0
    fprintf('Bad max transport relative position: %f, beat=%f\n',relpos,info.maxpll.getbeat());
    info.maxtransport
    relpos=0.0;
  end
  % Set pointer
  ldiff=abs(langle-relpos*2*pi+pi/2);
  ldiff(ldiff>pi)=abs(ldiff(ldiff>pi)-2*pi);
  [~,ptrindex]=min(ldiff);
  ptrwidth=8;
  ptr=max(1,ptrindex-ptrwidth):min(size(info.state,1),ptrindex+ptrwidth);
  info.state(ptr,:)=127;
  mid=floor(length(ptr)/2);
  for k=1:3
    info.mix(ptr,k)=[1:mid,length(ptr)-mid:-1:1]/mid;
  end
  info.mix(info.layout.outsider,:)=0;
end

% Map a 0.0-1.0 to [lo,hi] using exponential
function val=expcontrol(v,lo,hi)
  val=exp(v*log(hi/lo)+log(lo));
end

function v=invexpcontrol(val,lo,hi)
  v=log(val/lo)/log(hi/lo);
end