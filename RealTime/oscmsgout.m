% oscmsgout - send OSC message(s) to a particular destination
% Usage: msgout(ident,path,data,options)
% ident - destination code from urlconfig.txt
% path - message path
% data - cell array of data to send
% options - options followed by values
%   'debug' - true to log message using fprintf
function ok=oscmsgout(ident,path,data,varargin)
defaults=struct('debug',false,'log',true);
args=processargs(defaults,varargin);

global oscsetup;
ok=true;

if iscell(ident)
  % Process multiple destinations recursively
  for i=1:length(ident)
    ok=ok && oscmsgout(ident{i},path,data,'debug',args.debug);
  end
  return;
end

if isempty(oscsetup)
  oscinit();
end

if isempty(ident)
  % Send to all
  qi=1:length(oscsetup.clients);
else
  % Find specific entry
  qi=[];
  if ~isempty(oscsetup.clients)
    qi=find(strcmp(ident,{oscsetup.clients.ident}));
  end
  if isempty(qi)
    % Add new entry
    [host,port]=getsubsysaddr(ident);
    if isempty(port)
      ok=false;
      return;
    end
    oscadddest(sprintf('osc.udp://%s:%d',host,port),ident);
    qi=find(strcmp(ident,{oscsetup.clients.ident}));
  end
end

% Send message
toremove=[];
disabletime=30;
for i=1:length(qi)
  cl=oscsetup.clients(qi(i));
  if isfinite(cl.downsince)
    if (now-cl.downsince)*24*3600<60
      % Disabled
      continue;
    else
      fprintf('Retrying client %s@%s that was down\n', cl.ident, cl.url);
    end
  end
  ok=osc_send(cl.addr,struct('path',path,'data',{data}));
  if args.debug
    if isempty(cl.ident)
      nm=cl.url;
    else
      nm=cl.ident;
    end
    fprintf('%s<-%s\n', nm, formatmsg(path,data));
  end
  if args.log
    osclog('client',cl, 'path',path,'data',data);
  end

  if isfinite(cl.downsince)
    if ok
      % No longer down
      oscsetup.clients(qi(i)).downsince=nan;
      fprintf('Client %s@%s - osc_send succeeded\n', cl.ident,cl.url);
    else
      fprintf('Client %s@%s is still down\n', cl.ident,cl.url);
    end
  elseif ~ok
    fprintf('Failed send of message to OSC target %s@%s - disabling for %d seconds\n',cl.ident,cl.url,disabletime);
    oscsetup.clients(qi(i)).downsince = now;
  end
end
for i=length(toremove):-1:1
  oscrmdest(oscsetup.clients(i).url);
end