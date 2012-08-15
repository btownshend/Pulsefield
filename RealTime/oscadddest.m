% Add an OSC destination to global table
function oscadddest(url,ident)
if nargin<2
  error('oscaddest with 1 arg no longer supported');
end
global oscsetup;
if isempty(oscsetup) || ~isfield(oscsetup,'clients')
  oscinit();
end

for i=1:length(oscsetup.clients)
  cl=oscsetup.clients(i);
  if strcmp(cl.ident,ident)
    if strcmp(cl.url,url)
      fprintf('Already have OSC client %s@%s\n', ident, url);
      return;
    else
      fprintf('Removing superceded OSC client %s@%s\n', ident, cl.url);
      oscrmdest(ident);
    end
    break;
  end
end
fprintf('Adding OSC client %s@%s\n', ident,url);
addr=osc_new_address(url);
fprintf('done\n');
oscsetup.clients=[oscsetup.clients, struct('url',url,'addr',addr,'ident',ident,'downsince',nan)];

