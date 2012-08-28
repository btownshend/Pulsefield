% Add an OSC destination to global table
function oscadddest(url,ident)
global oscclients;

if nargin<2
  error('oscaddest with 1 arg no longer supported');
end

if isempty(oscclients)
  oscclients=struct('ident',{},'url',{},'addr',{},'downsince',nan);
end

for i=1:length(oscclients)
  cl=oscclients(i);
  if strcmp(cl.ident,ident)
    if strcmp(cl.url,url)
      fprintf('Already have OSC client %s@%s\n', ident, url);
      oscclients(i).downsince=nan;  % Re-enable
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
oscclients=[oscclients, struct('url',url,'addr',addr,'ident',ident,'downsince',nan)];

