% Frontend Control
% Usage: fectl(p,op)
% Op is 'start', 'quit','ping'
% Results is true for success, false for failure
function ok=felidarctl(p,op)
relpath='src/LIDAR/FrontEndLIDAR/frontend';
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
    cmd=sprintf('ssh %s "[ -r /tmp/frontend.log ] && mv /tmp/frontend.log /tmp/frontend.log.old; %s/%s %d >/tmp/frontend.log 2>&1 & sleep 2; cat /tmp/frontend.log; "', frontendhost, pfroot(), relpath, length(p.lidar));
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
