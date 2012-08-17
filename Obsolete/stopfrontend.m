function stopfrontend(p);
global visserver;

% Stop frontend by sending OSC quit signal
oscmsgout('FE',struct('path','/quit','data',{{}}));
fprintf('Sent /quit signal to front end\n');

if ~isempty(visserver)
  fprintf('Freeing OSC server at port %d\n', visserver.port);
  osc_free_server(visserver.addr);
  visserver=[];
end
