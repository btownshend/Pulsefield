% Frontend Control
% Usage: fectl(p,op)
% Op is 'start', 'quit','ping'
% Results is true for success, false for failure
function ok=fectl(p,op)
ok=true;   %Default as ok

% Handle list of ops in order recursively
if iscell(op)
  for i=1:length(op)
    ok=fectl(p,op{i});
    if ~ok
      fprintf('Stopping fectl processing due to failure of ''%s''\n', op{i});
      return;
    end
  end
  return;
end

fprintf('Running fectl(%s)\n', op);

% Make sure we have an OSC connection to receive messages (regardless of whether frontend is up)
if strcmp(op,'quit')
  % Stop frontend by sending OSC quit signal
  oscmsgout('FE','/quit',{});
  fprintf('Sent /quit signal to front end\n');
elseif strcmp(op,'start')
  % Launch frontend if needed, make sure connections OK
  running=fectl(p,'ping');
  if running
    fprintf('fectl(start): already running\n');
  else
    [frontendhost,~]=getsubsysaddr('FE');
    % Start frontend
    cmd=sprintf('ssh %s "[ -r /tmp/frontend.log ] && mv /tmp/frontend.log /tmp/frontend.log.old; %s/src/FrontEnd/frontend %d %d >/tmp/frontend.log 2>&1 & sleep 1; cat /tmp/frontend.log; "', frontendhost, pfroot(), length(p.camera), length(p.led));
    fprintf('Running cmd: "%s"\n', cmd);
    [s,r]=system(cmd);
    fprintf('System response: %s\n\n', r);
    if s~=0
      fprintf('Failed launch of frontend with cmd: ''%s'': %s\n',cmd, r);
      ok=false;
      return;
    end
    running=fectl(p,'ping');
  end
  if ~running
    fprintf('Front end failed to start\n');
    ok=false;
    return;
  end
  fprintf('Initializing frontend\n');

  % Initialize
  oscmsgout('FE','/vis/stop',{});
  oscmsgout('FE','/vis/set/fps',{p.analysisparams.fps});
  oscmsgout('FE','/vis/set/corrthresh',{0.5});
  for i=1:length(p.camera)
    c=p.camera(i);
    if strcmp(c.type,'av10115')
      res='full';
    elseif strcmp(c.type,'av10115-half')
      res='half';
    else
      fprintf('Unable to determine resolution of %s: assuming full res\n', c.type);
      res='full';
    end
    oscmsgout('FE','/vis/set/res',{int32(i-1),res});
    if isfield(p.camera(i),'viscache') && isfield(p.camera(i).viscache,'refim')
      oscmsgout('FE','/vis/set/updatetc',{ p.analysisparams.updatetc });    % Turn on updates of reference image
      fprintf('Frontend is automatically updating reference with a time constant of %f seconds\n', p.analysisparams.updatetc);
      refim = im2single(p.camera(i).viscache.refim);
      filename=sprintf('/tmp/setref_%.6f_%d.raw',(now-datenum(1970,1,1)+7/24)*24*3600,i);
      [fd,errmsg]=fopen(filename,'wb');
      if fd<0
        fprintf('Unable to open %s for writing: %s\n', filename,errmsg);
        ok=false;
        return;
      end
      fwrite(fd,permute(refim,[3,2,1]),'single');  % Write it in normal RGB order 
      fclose(fd);
      fprintf('Sending reference image to frontend in %s\n',filename);
      oscmsgout('FE','/vis/set/refimage',{ int32(i-1), size(refim,2), size(refim,1), size(refim,3), filename});
    else
      fprintf('Frontend is automatically updating reference with a time constant of %f seconds\n', p.analysisparams.updatetc);
      oscmsgout('FE','/vis/set/updatetc',{ p.analysisparams.updatetc });
    end
      
    % setRoi expects x0, y0, x1, y1 (with 1-based indexing)
    oscmsgout('FE','/vis/set/roi',{int32(i-1),int32(c.roi(1)),int32(c.roi(3)),int32(c.roi(2)),int32(c.roi(4))});
    
    for j=1:size(c.viscache.tlpos,1)
      tlpos=c.viscache.tlpos(j,:);
      brpos=c.viscache.brpos(j,:);
      if isnan(tlpos(1))
        oscmsgout('FE','/vis/set/pos',{int32(i-1),int32(j-1),int32(-1),int32(-1),int32(-1),int32(-1)});
      else
        oscmsgout('FE','/vis/set/pos',{int32(i-1),int32(j-1),int32(tlpos(1)-1),int32(tlpos(2)-1),int32(brpos(1)-tlpos(1)+1),int32(brpos(2)-tlpos(2)+1)});   % On the wire uses 0-based indexing
      end
    end
    pause(1);   % Try not to overrun UDP stack
  end
  oscmsgout('FE','/vis/start',{});
elseif strcmp(op,'ping')
  % Make sure FE knows where to send replies
  [~,myport]=getsubsysaddr('MPV');
  fprintf('Instructing frontend to use port %d to send us msgs\n', myport);
  oscmsgout('FE','/vis/dest/add/port',{myport});
  ok=oscping('FE','MPV');
else 
  fprintf('fectl: Unexpected op: %s\n', op);
end
