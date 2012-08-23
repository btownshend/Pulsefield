% Realtime run script

% Setup data structure
global recvis
if exist('recvis','var') && isfield(recvis,'vis') && ~isempty(recvis.vis) && ~isfield(recvis,'note') && length(recvis.vis)>=5
    z=input(sprintf('Are you sure you want to overwrite existing recvis with %d samples? [Y/N]: ',length(recvis.vis)),'s');
    if isempty(z) || upper(z)~='Y'
        return;
    end
end
recvis=struct('p',p,'vis',[],'tgtestimate',[],'possible',{{}},'randseed',rand());

% Save in well known location
save('/tmp/pulsefield_setup.mat','-struct','recvis');

osclog('close');   % Close any old logs open
oscclose();        % Close any old OSC connections

% Run sanity tests
if ~oscloopback('MPV') || ~oscloopback('MPO') || ~oscloopback('MPL') || ~oscloopback('MPA')
  fprintf('Loopback test failed\n');
  return;
end

% Setup destination for outgoing OSC messages
recvis.p.oscdests={'MAX','LD','TO'};

% Turn on LEDS directly (in case LED server is not running)
s1=arduino_ip(1);
setled(s1,-1,127*p.colors{1},1); show(s1); sync(s1);

% Turn on LED server if it is not already running
if ~lsctl(p,'start')
  error('Failed to start LED server');
end

% Make sure sensor cropping is OK
sensorcrop(p);

% Turn on LED Server
lsctl(p,'resume');

% Start front end if needed
if ~fectl(p,'ping') && ~fectl(p,'start')
  error('Failed to start front end');
end

% Wait for them to stabilize
pause(1);

fprintf('Ready\n');

samp=0;
starttime=now;
suppressuntil=0;
idlecnt=0;
prevsnap=[];
info=infoinit();
while ~info.quit
  if idlecnt==0
    timeout=0.0;
  else
    % Use a finite timeout if we've already done a refresh of other processes, so as not to hog all the CPU
    timeout=1/30;
  end
  vis=getvisible(recvis.p,'setleds',false,'timeout',timeout);

  if isempty(vis)
    % fprintf('No vis available\n');
    idlecnt=idlecnt+1;
  else
    samp=samp+1;
    if samp==1
      recvis.vis=vis;
    else
      recvis.vis=[recvis.vis,vis];
    end

    % Check for bad data
    transitions=sum(abs(diff(vis.v'))==1);
    if any(transitions>20) && samp>=suppressuntil
      suppressuntil=samp+50;
      fprintf('Too many LED blockage edges at snapshot %d: %s (ignoring until samp %d)\n', samp, sprintf('%d ',transitions),suppressuntil);
    end

    % Analyze data to estimate position of targets using layout
    snap=analyze(recvis.p,vis.v,false);
    snap.when=vis.when;
    snap.samp=samp;
    snap.idlecnt=idlecnt;
    idlecnt=0;
    
    if samp==1
      snap=inittgthypo(snap);
    else
      snap=updatetgthypo(recvis.p.layout,recvis.snap(samp-1),snap,samp);
    end

    snap.whendone=now;

    % Display coords
    %  fprintf('%3d@%4.1f ',samp,(now-starttime)*24*3600);

    ptd=false;
    for j=1:length(snap.tgts)
      tj=snap.tgts(j).pos;
      fprintf('T%d:(%.2f,%.2f) ',j,tj);
      ptd=true;
    end
    for j=1:length(snap.hypo)
      hj=snap.hypo(j);
      if samp>1 && length(recvis.snap(samp-1).hypo)>=j && recvis.snap(samp-1).hypo(j).id==hj.id
        hk=recvis.snap(samp-1).hypo(j);
      end
      fprintf('H%d:(%.2f,%.2f)@%.2f m/s ',hj.id,hj.pos,norm(hj.velocity));
      ptd=true;
    end
    if ptd
      fprintf(' (%d)\n',samp);
    end

    if ~isempty(prevsnap)
      % Make sure that if it is called twice without a vis update, that prevsnap==snap
      % TODO - break out update handling from incoming message handling
      info=oscupdate(recvis.p,info,samp,snap,prevsnap);
    else
      info=oscupdate(recvis.p,info,samp,snap);
    end
    prevsnap=snap;
  end
  
  info=oscincoming(recvis.p,info);
  
  if ~isempty(vis)
    snap.whendone2=now;
    recvis.snap(samp)=snap;
  end

  if info.needcal
    fprintf('Recalibrating...');
    [~,recvis.p]=getvisible(recvis.p,'init');
    info.needcal=false;
  end
end

% Turn off any remaining notes
oscupdate(recvis.p,info,samp);

% Turn off LEDs (in case LED Server not running)
setled(s1,-1,[0,0,0],1);show(s1);
