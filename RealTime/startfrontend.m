function ok=startfrontend(p);
global visserver

ok=false;  % By default

if isempty(visserver)
  running=false;
else
  oscmsgout('FE','/vis/stop',{});
  oscmsgout('FE','/ping',{});
  while true
    m=getnextfrontendmsg(1.0);
    if isempty(m)
      running=false;
      break;
    elseif strcmp(m.path,'/ack')
      fprintf('Front end already running -- got %s\n', m.path);
      running=true;
      break;
    else
      fprintf('Flush %s from frontend while looking for /ack\n',m.path);
    end
  end
end

[~,myport]=getsubsysaddr('MPV');

if ~running
  % Start frontend
  cmd=sprintf('../FrontEnd/frontend %d %d >/tmp/frontend.log 2>&1 & sleep 1; cat /tmp/frontend.log; ', length(p.camera), length(p.led));
  fprintf('Running cmd: "%s"\n', cmd);
  [s,r]=system(cmd);
  fprintf('%s\n',r);

  % Start a local server to receive vis messages
  if ~isempty(visserver)
    fprintf('Freeing old OSC server at port %d. ', visserver.port);
    osc_free_server(visserver.addr);
    visserver=[];
  end
  
  visserver=struct('port',myport,'addr',osc_new_server(myport),'msgin',[]);
  fprintf('Started new OSC server for /vis data at port %d\n', myport);
end

% Retrieve host,port and port for replies to me
[frontendhost,frontendport]=getsubsysaddr('FE');
fprintf('Instructing frontend at %s:%d to use port %d to send us msgs\n', frontendhost, frontendport, myport);
oscmsgout('FE','/vis/dest/add/port',{myport});

% Initialize data structures
rcvr(p,'init');   % Send 

fprintf('Sending /vis/start to front end;  waiting for /vis/started reply...');
oscmsgout('FE','/vis/start',{});

m=getnextfrontendmsg(1.0);
if isempty(m)
  error('No /started acknowledge received');
end

% Only waiting for started or stopped messages
if strcmp(m.path,'/vis/started')
  fprintf('Got acknowledge from frontend that it has started\n');
  ok=true;
elseif strcmp(m.path,'/vis/stopped')
  error('Front end replied with /stopped messages\n');
else
  fprintf('Unexpected message from frontend while waiting for /started: %s\n', m.path);
end
