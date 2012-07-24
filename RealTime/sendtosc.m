% sendtomax - send a message to max
% path - path (message name)
% data - cell array of data
function ok=sendtosc(path,data)
global ssc_addr;
if isempty(ssc_addr)
  sc_addr=osc_new_address('localhost',57120);
end
m=struct('path',path,'data',{data});
ok=osc_send(ssc_addr,m);
if ~ok
  fprintf('Failed send of message to SC\n');
end

