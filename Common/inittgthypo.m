% Initialize hypothesis of locations using fixed positions
% Input:
%   tgts - struct holding positions of targets
% Output:
%   h(1..n) - hypotheses for each individual
%       .pos(2) - position
%	.tnum - target number(s)
%	.like - likelihood
function h=inittgthypo(tgts)
h=[];
% Prob=1 that they are where they are
for i=1:size(tgts.tpos,1)
  if tgts.nunique(i)>10
    h=[h,struct('pos',tgts.tpos(i,:),'tnum',i,'like',1,'id',1,'entrytime',tgts.when,'lasttime',tgts.when)];
    fprintf('Initializing hypo %d to position of target %d - (%.1f, %.1f)\n', length(h), i, tgts.tpos(i,:));
  else
    fprintf('Initializing hypo: skip target %d - nunique=%d\n', i, tgts.nunique(i));
  end
end
