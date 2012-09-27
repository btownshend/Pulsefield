% Realtime run script
startdiary('realtime');
dosaves=1;   % 0-no saves, 1-by session, 2-all
extraframes=10;  % Number of frames to store before/after exit

% Setup data structure
global recvis
if ~exist('p','var')
  if exist('recvis','var') && isfield(recvis,'p')
    fprintf('Loading p from recvis\n');
    p=recvis.p;
  else
    fprintf('Loading p from Calibration/current.mat\n');
    p=load('Calibration/current.mat');
  end
end

if exist('recvis','var') && isfield(recvis,'vis') && ~isempty(recvis.vis) && ~isfield(recvis,'note') && length(recvis.vis)>extraframes
    z=input(sprintf('Are you sure you want to overwrite existing recvis with %d samples? [Y/N]: ',length(recvis.vis)),'s');
    if isempty(z) || upper(z)~='Y'
        return;
    end
end
recvis=struct('p',p,'vis',[],'randseed',rand());

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
maxoccupancy=0;
prevsnap=[];
info=infoinit();

photointerval=10.0;   % Take a photo ever 10 seconds while someone present
lastphoto=0;
postbuffering=0;   % Number of frames PF has been empty

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
    if 0 && length(prevsnap.hypo)>0 && (now-lastphoto)*3600*24>photointerval
      fprintf('Taking a snapshot photo\n');
      snapshot(p);
      lastphoto=now;
    end
  else
    info.health.gotmsg('FE');
    samp=samp+1;
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
      snap=updatetgthypo(recvis.p.layout,prevsnap,snap,samp);
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
      if samp>1 && length(prevsnap.hypo)>=j && prevsnap.hypo(j).id==hj.id
        hk=prevsnap.hypo(j);
      end
      fprintf('H%d:(%.2f,%.2f)@%.2f m/s ',hj.id,hj.pos,norm(hj.velocity));
      ptd=true;
    end
    if length(snap.hypo)>maxoccupancy
      maxoccupancy=length(snap.hypo);
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

    if length(snap.hypo)==0
      % Keep count of how many frames its been empty to know when to save recording
      postbuffering=postbuffering+1;
    else
      % Occupied
      postbuffering=0;
    end
    
    if dosaves==1 && postbuffering==extraframes && maxoccupancy>0
      % Just emptied out, save data
      tic;
      fprintf('All empty for %d frames after max occupancy of %d - saving %d frames\n', postbuffering, maxoccupancy, length(recvis.vis));
      try
        saverecvis(recvis,sprintf('Auto-save of %d frames at %s with max occupancy of %d',length(recvis.vis),datestr(now),maxoccupancy));
      catch me
        fprintf('saverecvis: %s\n',me.message);
      end
      maxoccupancy=0;
      
      savetime=toc;
      fprintf('Save took %.2f seconds\n',savetime);

      % New diary
      startdiary();
      startdiary('realtime');
    end

    if ~isempty(vis)
      snap.whendone2=now;
    end

    if dosaves>0
      if ~isfield(recvis,'snap')
        % Start saving frames
        recvis.snap=snap;
        recvis.vis=vis;
      elseif maxoccupancy>0 || dosaves==2 || length(recvis.snap)<extraframes
        % Save new frame
        recvis.snap(end+1)=snap;
        recvis.vis(end+1)=vis;
      else
        % Just keep most recent extraframes to have prebuffering
        recvis.snap=[recvis.snap((end-extraframes+2):end),snap];
        recvis.vis=[recvis.vis((end-extraframes+2):end),vis];
      end
    end
    %    fprintf('Retaining %d frames\n',length(recvis.snap));

    prevsnap=snap;
  end
  info=oscincoming(recvis.p,info);
  

  if info.needcal>0
    if info.needcal>1
      fprintf('Full calibration.  LED localization...');
      oscmsgout('TO','/pf/reset/color',{'yellow'});
      recvis.p=pixcalibrate(recvis.p);
      oscmsgout('TO','/pf/reset/color',{'red'});
      fprintf('done ');
    end
    fprintf('Correlations...');
      oscmsgout('TO','/pf/calibrate/color',{'yellow'});
    [~,recvis.p]=getvisible(recvis.p,'init');
    oscmsgout('TO','/pf/calibrate/color',{'red'});
    fprintf('done\n');
    info.needcal=0;
    savecalib(p);
  end
end

% Turn off any remaining notes
oscupdate(recvis.p,info,samp);

% Turn off LEDs (in case LED Server not running)
setled(s1,-1,[0,0,0],1);show(s1);
