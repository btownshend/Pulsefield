% oscmsgin - retrieve next message from server
% Usages m=oscmsgin(ident,timeout,peek)
%  timeout - timeout to wait in seconds (default 0.0)
%  peek - true to return next message already read without removing from queue
function msg=oscmsgin(ident,timeout,peek)
global oscservers;
debug=0;
log=0;

if nargin<2
  timeout=0.0;
end
if nargin<3
  peek=false;
end

msg=[];   % Default result

if isempty(oscservers)
  svrind=[];
else
  svrind=find(strcmp(ident,{oscservers.ident}));
  assert(length(svrind)<=1);
end

if isempty(svrind)
  [~,port]=getsubsysaddr(ident);
  if isempty(port)
    fprintf('oscmsgin: Bad ident: %s\n', ident);
    return;
  end
  try
    addr=osc_new_server(port);
  catch me
    error('Unable to create server on port %d for %s: %s\n', port, ident, me.message);
  end
  oscservers=[oscservers,struct('ident',ident,'port',port,'addr',addr,'msgqueue',{{}})];
  svrind=length(oscservers);
  fprintf('Opened OSC server %s on port %d at %s\n', ident, port, addr);
end
  
if peek
  % Peek ahead
  msg=oscservers(svrind).msgqueue;
  return;   % Return empty result if nothing already in queue
end

if isempty(oscservers(svrind).msgqueue)
  % Get some more messages, timeout after given time
  oscservers(svrind).msgqueue=osc_recv(oscservers(svrind).addr,timeout);
  if ~iscell(oscservers(svrind).msgqueue)
    % osc_recv returns 1 entry as a struct instead of a cell array
    if debug
      fprintf('Restructured message\n');
    end
    oscservers(svrind).msgqueue={oscservers(svrind).msgqueue};
  end
  % Is it still empty?
  if isempty(oscservers(svrind).msgqueue)
    if debug
      if timeout>0
        fprintf('No messages available after timeout of %.2f (%.2f) seconds\n', timeout,el);
      else
        fprintf('No messages available\n');
      end
    end
    return;
  end
end

msg=oscservers(svrind).msgqueue{1};
if log
  osclog('msg',msg,'server',ident);
end
oscservers(svrind).msgqueue=oscservers(svrind).msgqueue(2:end);

if debug
  fprintf('oscmsgin(%s): Have %d messages, next is %s\n', ident, length(oscservers(svrind).msgqueue)+1, formatmsg(msg.path,msg.data));
end
