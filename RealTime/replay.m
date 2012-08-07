% Replay a recorded set of movements stored in recvis
function replayrecvis=replay(varargin)
defaults=struct('timed',false,'plots',{{}},'oscdests',{{'SG','LD','TO'}});
args=processargs(defaults,varargin);

global recvis
if ~exist('recvis','var') || isempty(recvis.vis)
    error('Nothing to replay -- need to load recvis');
end

fprintf('Replaying session from %s to %s\n', datestr(recvis.vis(1).when),datestr(recvis.vis(end).when));
if isfield(recvis,'note')
  fprintf('Notes: %s\n',recvis.note);
end

timedreplay=args.timed;   % Set to 1 to replay at same pacing as recording
plots=args.plots;

% Setup destination for outgoing OSC messages
for i=1:length(oscdests)
  [h,p]=getsubsysaddr(oscdests{i});
  if ~isempty(h)
    oscadddest(['osc.udp://',h,':',num2str(p)])
  end
end
mainloop

% Copy in snap to new version
fprintf('Copying new analysis into replayrecvis\n');
replayrecvis=recvis;
for i=1:length(snap)
  replayrecvis.snap(i)=snap{i};
end
plothypo(replayrecvis);
plotlatency(replayrecvis);
plotstats(replayrecvis);
% Can run analyze(recvis.p,recvis.vis(nnn).v,2) to plot