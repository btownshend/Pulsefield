% Control Max
% op - 'ping'
function ok=maxctl(p,op,varargin)
ok=true;
if strcmp(op,'ping')
  % Make sure MAX knows where to send replies
  myident='MPO';
  [~,myport]=getsubsysaddr(myident);
  fprintf('Instructing max to use port %d to send us msgs\n', myport);
  oscmsgout('MAX','/pf/dest/add/port',{myport,myident});
  % Ping
  ok=oscping('MAX',myident);
else
  fprintf('Unknown op: %s\n', op);
  ok=false;
end
