% Update current hyothesis of locations 
% Input:
%   h - prior hypotheses
%   tgts - target structure for current sample
%   maxmovement - max movement of anyone in m
% Output:
%   hf - updated hypotheses
function hf=updatetgthypo(p,layout,imap,h,tgts,maxmovement)
% Look at certain targets first
minunique=5;  % At least this many rays are only blocked by this target
droptime=1;   % Time to drop a missed target in seconds
addtime=1.1;    % Time to add a new spontaneous target
ntgts=size(tgts.tpos,1);
if length(h)==0
  nextid=1;
else
  nextid=max([h.id]+1);
end
dist=[];
like=[];
for i=1:ntgts
  for j=1:length(h)+1
    if j<=length(h)
      dist(i,j)=norm(tgts.tpos(i,:)-h(j).pos);
    else
      % Add a new possible hypothesis corresponding to an entry
      dist(i,j)=norm(tgts.tpos(i,:)-layout.entry);
    end
    % Estimate of likelihood that target i is the same as hypo j (heuristic)
    % Weight inversely by distance 
    like(i,j)=((dist(i,j)<maxmovement)+.01)* (maxmovement/(maxmovement+dist(i,j))) ;
    if j<=length(h)
      % Decrease likelihood of missing ones linearly with time since last seen
      like(i,j)=like(i,j)*(droptime+1)/(droptime+1-(tgts.when-h(j).lasttime)*24*3600);
      % Decrease likelihood for new ones
      existent=(h(j).lasttime-h(j).entrytime)*24*3600;
      if existent<addtime
        like(i,j)=like(i,j)/2;
      end
    end
  end
  % Decrease likelhood of targets that don't have unique rays
  if tgts.nunique(i)<minunique
    like(i,:)=like(i,:)/4;
  end
end

% Assign by highest likelihood
hf=[];
matched=zeros(1,ntgts);
while any(~matched & tgts.nunique>=minunique)
  [~,mpos]=max(like(:));
  [mi,mj]=ind2sub(size(like),mpos);
  if mj<=length(h) && (isempty(hf) || ~ismember(h(mj).id,[hf.id]))
    id=h(mj).id;
    entrytime=h(mj).entrytime;
    spd=norm(tgts.tpos(mi,:)-h(mj).pos)/((tgts.when-h(mj).lasttime)*3600*24);
  else
    % More than one target for this hypo (or a new entry)
    id=nextid;
    nextid=nextid+1;
    entrytime=tgts.when;
    spd=nan;
    if mj>length(h)
      fprintf('New entry with hypo %d\n', id);
    else
      fprintf('Multiple targets correspond to prior hypothesis %d - adding hypo %d\n',h(mj).id, id);
    end
  end
  hf=[hf,struct('id',id,'pos',tgts.tpos(mi,:),'tnum',mi,'like',like(mi,mj),'entrytime',entrytime,'lasttime',tgts.when,'area',tgts.area(mi),'speed',spd,'minoraxislength',tgts.minoraxislength(mi),'majoraxislength',tgts.majoraxislength(mi))];
  fprintf('Assigned target %d (%.1f,%.1f) to hypo %d with dist=%.1f, like=%.2f\n', mi, tgts.tpos(mi,:), id, dist(mi,mj), like(mi,mj));
  like(mi,:)=like(mi,:)/10;  % Unlikely that this target is another hypothesis (convergence of targets)
  like(:,mj)=like(:,mj)/10;  % Unlikely that this hypo is another target (divergence of targets)
  like(mi,mj)=0;
  matched(mi)=1;
end
for i=1:length(h)
  if isempty(hf) || ~ismember(h(i).id,[hf.id])
    % No target assigned
    lostdur=tgts.when-h(i).lasttime;
    fprintf('No target assigned to hypothesis %d (lost for %.1f seconds)\n', h(i).id, lostdur*24*3600);
    if lostdur*24*3600>droptime
      fprintf('Dropping hypothesis %d after unseen for %.1f seconds\n', h(i).id,lostdur*24*3600);
    else
      hf=[hf,h(i)];
      hf(end).speed=nan;
      hf(end).area=nan;
    end
  end
end
missed=find(~matched & tgts.nunique>minunique);
for i=1:length(missed)
  fprintf('Target %d at (%.1f,%.1f) with %d unique rays not matched\n', missed(i), tgts.tpos(missed(i),:), tgts.nunique(missed(i)));
  keyboard
end
