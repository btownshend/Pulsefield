% oscmsgout - send OSC message(s) to a particular destination
% Usage: msgout(ident,path,data,debug,log)
% ident - destination code from urlconfig.txt
% path - message path
% data - cell array of data to send
% debug - true to turn on debugging (default: false)
% log - true to log message using osclog() (default: false)
function ok=oscmsgout(ident,path,data,debug,log)
if nargin<4
  debug=false;
end
if nargin<5
  log=false; 
end

global oscclients;
ok=true;

if iscell(ident)
  % Process multiple destinations recursively
  for i=1:length(ident)
    ok=ok && oscmsgout(ident{i},path,data,debug,log);
  end
  return;
end

% Find specific entry
qi=[];
if ~isempty(oscclients)
  qi=find(strcmp(ident,{oscclients.ident}));
end
if isempty(qi)
  % Add new entry
  [host,port]=getsubsysaddr(ident);
  if isempty(port)
    ok=false;
    return;
  end
  oscadddest(sprintf('osc.udp://%s:%d',host,port),ident);
  qi=find(strcmp(ident,{oscclients.ident}));
end

% Send message
toremove=[];
disabletime=15;
for i=1:length(qi)
  cl=oscclients(qi(i));
  if isfinite(cl.downsince)
    if (now-cl.downsince)*24*3600<disabletime
      % Disabled
      continue;
    else
      fprintf('Retrying client %s@%s that was down %d seconds ago\n',cl.ident, cl.url, (now-cl.downsince)*24*3600);
    end
  end
  ok=osc_send(cl.addr,struct('path',path,'data',{data}));
  if debug
    if isempty(cl.ident)
      nm=cl.url;
    else
      nm=cl.ident;
    end
    fprintf('%s<-%s\n', nm, formatmsg(path,data));
  end
  if log
    osclog('client',cl, 'path',path,'data',data);
  end

  if isfinite(cl.downsince)
    if ok
      % No longer down
      oscclients(qi(i)).downsince=nan;
      fprintf('Client %s@%s - osc_send succeeded\n', cl.ident,cl.url);
    else
      fprintf('Client %s@%s is still down\n', cl.ident,cl.url);
      oscclients(qi(i)).downsince = now;
    end
  elseif ~ok
    fprintf('Failed send of message to OSC target %s@%s - disabling for %d seconds\n',cl.ident,cl.url,disabletime);
    oscclients(qi(i)).downsince = now;
  end
end
for i=length(toremove):-1:1
  oscrmdest(oscclients(i).url);
end