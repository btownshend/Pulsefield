% Close OSC connections
function oscclose
global oscsetup

if isempty(oscsetup)
  fprintf('Nothing to close; oscsetup not found\n');
  return
end

% Need to close existing clients and server
for c=1:length(oscsetup.clients)
  cl=oscsetup.clients{c};
  osc_free_address(cl.addr);
  fprintf('Closed connection to client at %s:%d\n', cl.host,cl.port);
end
oscsetup.clients=[];

fprintf('Closing existing OSC server port\n');
osc_free_server(oscsetup.server);

clear global oscsetup

