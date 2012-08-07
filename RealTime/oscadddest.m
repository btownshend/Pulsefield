% Add an OSC destination
function oscadddest(url)
global oscsetup;
if isempty(oscsetup) || ~isfield(oscsetup,'clients')
  oscinit();
end

for i=1:length(oscsetup.clients)
  cl=oscsetup.clients(i);
  if strcmp(cl.url,url)
    fprintf('Already have OSC client at %s\n', url);
    return;
  end
end
addr=osc_new_address(url);
oscsetup.clients=[oscsetup.clients, struct('url',url,'addr',addr)];
fprintf('Added OSC client at %s\n', url);

