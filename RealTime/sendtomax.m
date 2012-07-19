% sendtomax - send a message to max
% path - path (message name)
% data - cell array of data
function ok=sendtomax(path,data)
global osc_addr;
if isempty(osc_addr)
  osc_addr=osc_new_address('192.168.0.141',7000);
end
m=struct('path',path,'data',{data});
ok=osc_send(osc_addr,m);
if ~ok
  fprintf('Failed send of message to Max\n');
end

