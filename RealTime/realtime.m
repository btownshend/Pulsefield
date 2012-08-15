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
oscclose();        % Close any old clients
getsubsysaddr([],'reload',true);   % Reload subsystem addresses
oscdests={'MAX','LD','TO'};

% Setup destination for outgoing OSC messages
for i=1:length(oscdests)
  [host,port]=getsubsysaddr(oscdests{i});
  if ~isempty(host)
    oscadddest(['osc.udp://',host,':',num2str(port)],oscdests{i})
  end
end

% Turn on LEDS directly (in case LED server is not running)
s1=arduino_ip(1);
setled(s1,-1,127*p.colors{1},1); show(s1); sync(s1);

% Make sure sensor cropping is OK
sensorcrop(p);

% Turn on LED Server
oscmsgout('LD','/led/start',{});

% Wait for them to stabilize
pause(1);

fprintf('Ready\n');

samp=0;
starttime=now;
suppressuntil=0;
flags=struct('needcal',false,'quit',false);
while ~flags.quit
  samp=samp+1;
  vis=getvisible(recvis.p,'setleds',false);

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
    fprintf('\n');
  end
  
  if samp>1
    flags=oscupdate(recvis.p,samp,snap,recvis.snap(samp-1));
  else
    flags=oscupdate(recvis.p,samp,snap);
  end

  recvis.snap(samp)=snap;

  if flags.needcal
    fprintf('Recalibrating...');
    [~,recvis.p]=getvisible(recvis.p,'init');
    flags.needcal=false;
  end

  % LED updates handled by LedServer now
  % updateleds(recvis.p,snap);
  % visleds(recvis.p,vis);
end

% Turn off any remaining notes
oscupdate(recvis.p,samp);

% Turn off LEDs (in case LED Server not running)
setled(s1,-1,[0,0,0],1);show(s1);
