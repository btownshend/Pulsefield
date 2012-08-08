% oscmsgout - send OSC message(s) to a particular destination
% Usage: msgout(ident,path,data,options)
% ident - destination code from urlconfig.txt
% path - message path
% data - cell array of data to send
% options - options followed by values
%   'debug' - true to log message using fprintf
function ok=oscmsgout(ident,path,data,varargin)
defaults=struct('debug',false);
args=processargs(defaults,varargin);

global oscsetup;
ok=true;

if iscell(ident)
  % Process multiple destinations recursively
  for i=1:length(ident)
    ok=ok && msgout(ident{i},path,data);
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
for i=1:length(qi)
  cl=oscsetup.clients(qi(i));
  ok=osc_send(cl.addr,struct('path',path,'data',{data}));
  if args.debug
    fprintf('Sending %s to %s\n', formatmsg(path,data),cl.url);
  end
  if ~ok
    fprintf('Failed send of message to OSC target at %s - removing\n',cl.url);
    toremove=[toremove,qi(i)];
  end
end
for i=length(toremove):-1:1
  oscrmdest(oscsetup.clients(i).url);
end
