% Add an OSC destination to global table
function oscadddest(url,ident)
if nargin<2
  ident=[];
end
global oscsetup;
if isempty(oscsetup) || ~isfield(oscsetup,'clients')
  oscinit();
end

for i=1:length(oscsetup.clients)
  cl=oscsetup.clients(i);
  if strcmp(cl.url,url)
    if isempty(cl.ident) && ~isempty(ident)
      fprintf('Updating OSC client at %s with ident (%s)\n', url, ident);
      oscsetup.clients(i).ident=ident;
    else
      fprintf('Already have OSC client (%s) at %s\n', ident, url);
    end
    return;
  end
end
fprintf('Adding OSC client (%s) at %s\n', ident,url);
addr=osc_new_address(url);
fprintf('done\n');
oscsetup.clients=[oscsetup.clients, struct('url',url,'addr',addr,'ident',ident)];

