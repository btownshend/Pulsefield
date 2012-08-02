% Add an OSC destination
function oscadddest(host,port)
global oscsetup;
if isempty(oscsetup) || ~isfield(oscsetup,'clients')
  oscinit();
end
for i=1:length(oscsetup.clients)
  cl=oscsetup.clients(i);
  if strcmp(cl.host,host) && cl.port==port
    fprintf('Already have OSC client at %s:%d\n', host,port);
    return;
  end
end
addr=osc_new_address(host,port);
oscsetup.clients=[oscsetup.clients, struct('host',host,'port',port,'addr',addr)];
fprintf('Added OSC client at %s:%d\n', host, port);

