% Control separate led processes
% op - 'start', 'pause', 'resume', 'quit'
function ok=lsctl(p,op,varargin)
ok=true;
if strcmp(op,'start')
  running=oscping('LD','MPL');
  if running
    fprintf('LedServer already running\n');
  else
    pfilename='/tmp/pfile.mat';
    save(pfilename,'-struct','p');
    cmd=sprintf('/Applications/MATLAB_R2012b.app/bin/matlab -nodesktop -nosplash -r "p=load(''%s''); ledserver(p); quit;" >/tmp/ledserver.log 2>&1 &',pfilename);
    fprintf('Running command: %s\n', cmd);
    [r,s]=system(cmd);
    if s~=0
      fprintf('Failed launch of LED server: %s\n', r);
      ok=false;
      return;
    end
    fprintf('Waiting for startup...');
    for i=1:10
      fprintf('%d..',i);
      running=oscping('LD','MPL');
      if running
        break;
      end
      pause(1);
    end
    fprintf('Log file:\n');
    !cat /tmp/ledserver.log
  end
  if ~running
    fprintf('LedServer failed to start\n');
    ok=false;
    return;
  end
elseif strcmp(op,'pause')
  fprintf('Sending PAUSE command to LedServer\n');
  ok=oscmsgout('LD','/led/pause',{});
elseif strcmp(op,'resume')
  fprintf('Sending RESUME command to LedServer\n');
  ok=oscmsgout('LD','/led/resume',{});
elseif strcmp(op,'quit')
  fprintf('Sending QUIT command to LedServer\n');
  ok=oscmsgout('LD','/quit',{});
elseif strcmp(op,'ping')
  % Make sure LEDServer knows where to send replies
  [~,myport]=getsubsysaddr('MPL');
  fprintf('Instructing ledserver to use port %d to send us msgs\n', myport);
  oscmsgout('LD','/pf/dest/add/port',{myport,'MPL'});
  ok=oscping('LD','MPL');
else
  fprintf('Unknown op: %s\n', op);
  ok=false;
end
