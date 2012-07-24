global recvis
if ~exist('recvis') || isempty(recvis.vis)
    error('Nothing to replay -- need to load recvis');
end
timedreplay=0;   % Set to 1 to replay at same pacing as recording
plots={'hypo'};
fprintf('Replaying session from %s to %s\n', datestr(recvis.vis(1).when),datestr(recvis.vis(end).when));
if isfield(recvis,'note')
  fprintf('Notes: %s\n',recvis.note);
end
mainloop
plotlatency(recvis);

