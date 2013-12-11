% Initialize hypothesis of locations using fixed positions
% Input:
%   snap -
%     tgts(1..m) - struct holding positions of targets
% Output:
%   snap - 
%     h(1..n) - hypotheses for each individual
%       .pos(2) - position
%	.tnum - target number(s)
%	.like - likelihood
%     like(m+1,n+1) - raw likelihoods of target i matching hypo j;  like(i,n)~like of entry; like(m,j)~like of exit
function snap=inittgthypo(snap)
h=struct('id',{},'pos',{},'tnum',{},'like',{},'entrytime',{},'lasttime',{},'area',{},'velocity',{},'heading',{},'orientation',{},'minoraxislength',{},'majoraxislength',{},'groupid',{},'groupsize',{});
tgts=snap.tgts;
for i=1:length(tgts)
  if tgts(i).nunique>10
    h=[h,struct('pos',tgts(i).pos,'tnum',i,'like',1,'id',length(h)+1,'entrytime',snap.when,'lasttime',snap.when,'area',nan,'velocity',[nan,nan],'heading',nan,'orientation',nan,'minoraxislength',nan,'majoraxislength',nan,'groupid',0,'groupsize',1)];
    fprintf('Initializing hypo %d to position of target %d - (%.1f, %.1f)\n', length(h), i, tgts(i).pos);
  else
    fprintf('Initializing hypo: skip target %d - nunique=%d\n', i, tgts(i).nunique);
  end
end
snap.nextid=length(h)+1;
snap.hypo=h;
snap.like=zeros(length(tgts)+1,length(h)+1);