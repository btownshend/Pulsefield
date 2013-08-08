% Summarize which LEDs are blocked in a vis structure
function blocked(vis)
fprintf('Total blocked: %d\n',sum(vis.v(:)==0));
for c=1:size(vis.v,1)
  fprintf('Camera %d: %s\n', c, shortlist(find(vis.v(c,:)==0)));
end
