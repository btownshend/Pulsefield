% Close all OSC connections
function oscclose
global oscservers

if isempty(oscservers)
  fprintf('No servers to close\n');
end

for i=1:length(oscservers)
  fprintf('Closing %s server on port %d\n', oscservers(i).ident, oscservers(i).port);
  osc_free_server(oscservers(i).addr);
end

oscservers=[];

% Close all clients
oscrmdest();
