function stopfrontend(p);
global visserver;

% Stop frontend by sending OSC quit signal
[frontendhost,frontendport]=getsubsysaddr('FE');
addr=osc_new_address(frontendhost,frontendport);
osc_send(addr,struct('path','/quit','data',{{}}));
osc_free_address(addr);
fprintf('Sent /quit signal to front end at %s:%d\n', frontendhost, frontendport);

if ~isempty(visserver)
  fprintf('Freeing OSC server at port %d\n', visserver.port);
  osc_free_server(visserver.addr);
  visserver=[];
end
