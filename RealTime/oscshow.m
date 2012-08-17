% oscshow - show all connections
function oscshow
global oscclients oscservers;
for i=1:length(oscservers)
  fprintf('OSC Server %s on port %d at %s with %d messages in queue.\n', oscservers(i).ident, oscservers(i).port, oscservers(i).addr,length(oscservers(i).msgqueue));
end

for i=1:length(oscclients)
  fprintf('OSC Client %s@%s at %s (downsince=%f)\n', oscclients(i).ident, oscclients(i).url, oscclients(i).addr,oscclients(i).downsince);
end