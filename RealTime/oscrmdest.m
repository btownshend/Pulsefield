% Remove an OSC destination
function oscrmdest(urlOrIdent)
global oscsetup;
if isempty(oscsetup) || ~isfield(oscsetup,'clients')
  return;
end

if nargin==0
  if ~isempty(oscsetup.clients)
    fprintf('Removed all %d OSC clients\n',length(oscsetup.clients));
    oscsetup.clients=[];
  end
  return;
end

clients=oscsetup.clients;
toremove=zeros(1,length(clients));
for c=1:length(clients)
  if strcmp(clients(c).url,urlOrIdent) || strcmp(clients(c).ident,urlOrIdent)
    toremove(c)=1;
    osc_free_address(clients(c).addr);
  end
end
if ~any(toremove)
  fprintf('Unable to remove OSC destination %s - not found\n', urlOrIdent);
else
  fprintf('Removed OSC client %s@%s\n',sum(toremove),clients(c).ident,clients(c).url);
  oscsetup.clients=clients(~toremove);
end
