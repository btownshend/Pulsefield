% Remove an OSC destination
function oscrmdest(urlOrIdent)
global oscclients;
if isempty(oscclients)
  return;
end

toremove=zeros(1,length(oscclients));
for c=1:length(oscclients)
  if nargin==0 || strcmp(oscclients(c).url,urlOrIdent) || strcmp(oscclients(c).ident,urlOrIdent)
    toremove(c)=1;
    osc_free_address(oscclients(c).addr);
    fprintf('Removed OSC client %s@%s\n',oscclients(c).ident,oscclients(c).url);
  end
end
if any(toremove)
  oscclients=oscclients(~toremove);
else
  fprintf('Unable to remove OSC destination %s - not found\n', urlOrIdent);
end
