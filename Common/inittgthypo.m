% Initialize hypothesis of locations using fixed positions
% Input:
%   tgts - struct holding positions of targets
% Output:
%   h(1..n) - hypotheses for each individual
%       .pos(2) - position
%	.tnum - target number(s)
%	.like - likelihood
function snap=inittgthypo(snap)
h=struct('id',{},'pos',{},'tnum',{},'like',{},'entrytime',{},'lasttime',{},'area',{},'delta',{},'speed',{},'heading',{},'orientation',{},'minoraxislength',{},'majoraxislength',{});
tgts=snap.tgts;
for i=1:length(tgts)
  if tgts(i).nunique>10
    h=[h,struct('pos',tgts(i).pos,'tnum',i,'like',1,'id',1,'entrytime',snap.when,'lasttime',snap.when,'area',nan,'delta',nan,'speed',nan,'heading',nan,'orientation',nan,'minoraxislength',nan,'majoraxislength',nan)];
    fprintf('Initializing hypo %d to position of target %d - (%.1f, %.1f)\n', length(h), i, tgts(i).pos);
  else
    fprintf('Initializing hypo: skip target %d - nunique=%d\n', i, tgts(i).nunique);
  end
end
snap.nextid=length(h)+1;
snap.hypo=h;
snap.like=[];