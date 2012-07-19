% Update current hypothesis of locations 
% Input:
%   h - prior hypotheses
%   snap - snapshot including target structure for current sample
% Output:
%   snap - w/hypo field added, snap.nextid updated
function snap=updatetgthypo(layout,prevsnap,snap)
debug=0;
% PARAMETERS
minunique=5;  % Could be ghost if less than this many unique rays
pghost=0.9;	% with this prob
droptime=5;   % Time to drop a missed target in seconds
speedmu=0.5;	% Mu of exponential distribution for PDF of speeds expected (m/s)
entryrate=1/30;	% 1 entries/30 seconds - also exponential (Poisson) distribution (entries/second)
exitrate=entryrate;
falselifetime=1;  % Mean lifetime of false signals (seconds)
muhidden=1;     % Mean lifetime of hidden people (not appearing in any target, but still present)
minimumlike=.001;  % Minimum likelihood (prob) to accept a matchup
poserror=0.1;    % Mean position error
maxpersons=4;     % Maximum undivided group size
meanpersonarea=0.11;   % In m^2
stdpersonarea=0.04;    % Stddev of person area

tgts=snap.tgts;
ntgts=length(tgts);
h=prevsnap.hypo;  % Start off by continuing old hypotheses
snap.nextid=prevsnap.nextid;
dt=(snap.when-prevsnap.when)*24*3600;

% Calculate probability that each target is multiple people
pmultiple=zeros(ntgts,maxpersons);
for i=1:ntgts
  for j=1:maxpersons
    pmultiple(i,j)=normpdf(tgts(i).area,meanpersonarea*j,stdpersonarea*sqrt(j));
  end
  pmultiple(i,:)=pmultiple(i,:)/sum(pmultiple(i,:));
%  like(i,:)=like(i,:)*dot(pmultiple(i,:),1:maxpersons);
end

dist=nan(ntgts+1,length(h)+1);
like=dist;
newpos={};
for i=1:ntgts
  for j=1:length(h)+1
    if j<=length(h)
      % Find distance to all pixels of target
      pixdist=sqrt((tgts(i).pixellist(:,1)-h(j).pos(:,1)).^2+(tgts(i).pixellist(:,2)-h(j).pos(:,2)).^2);
      % Use closest pixel position as new position
      [dist(i,j),closest]=min(pixdist);
      newpos{i,j}=tgts(i).pixellist(closest,:);
%      dist(i,j)=norm(tgts(i).pos-h(j).pos);
      dist(i,j)=max(0,dist(i,j)-tgts(i).majoraxislength/4);
      % Weight inversely by distance 
      tlastseen=(snap.when-h(j).lasttime)*24*3600;
      pspeed=exppdf(dist(i,j)/tlastseen,speedmu);
      pdir=poserror/max(poserror,dist(i,j));   % Prob of heading in that direction
      pentry=1;
      % Decrease likelihood of missing ones linearly with time since last seen
      phidden=exppdf(tlastseen,muhidden);
      % Decrease likelihood for new ones
      existent=(snap.when-h(j).entrytime)*24*3600;
      pfalse=1-expcdf(existent,falselifetime);   % Probability of this being a false signal
    elseif j==length(h)+1
      % Add a new possible hypothesis corresponding to an entry
      pixdist=sqrt((tgts(i).pixellist(:,1)-layout.entry(1)).^2+(tgts(i).pixellist(:,2)-layout.entry(2)).^2);
      [dist(i,j),closest]=min(pixdist);
      newpos{i,j}=tgts(i).pixellist(closest,:);

      % dist(i,j)=norm(tgts(i).pos-layout.entry);  % Distance from entry
      pspeed=exppdf(dist(i,j)/(dt/2),speedmu);   % Prob they moved to here if they just entered
      pdir=1;
      pentry=exppdf(dt,entryrate);   % Prob they entered in last dt seconds
      phidden=1;
      pfalse=0;
    end
    % Estimate of likelihood that target i is the same as hypo j (heuristic)
    like(i,j)=pspeed*pdir*pentry*(1-pfalse)*phidden;
  end
  % Decrease likelhood of targets that don't have unique rays
  if tgts(i).nunique<minunique
    like(i,:)=like(i,:)*(1-pghost);
  end
end

% Add in row for entries
i=ntgts+1;
for j=1:length(h)
  % Add a new possible hypothesis corresponding to an exit
  dist(i,j)=norm(h(j).pos-layout.entry);  % Distance from entry
  pspeed=exppdf(dist(i,j)/(dt/2),speedmu);   % Prob they moved to here if they just entered
  pentry=exppdf(dt,exitrate);   % Prob they entered in last dt seconds
  phidden=1;
  pfalse=0;
  like(i,j)=pspeed*pentry*(1-pfalse)*phidden;
end


if debug && size(like,1)>0
  fprintf('\n  Uniq    XPos   YPos  ');
  for j=1:size(like,2)-1
    fprintf('  H%-3d  ',h(j).id);
  end
  fprintf(' Enter\n');
  for i=1:size(like,1)
    if i==size(like,1)
      fprintf('Exit    ');
    else
      fprintf('T%-2d %3d %6.3f %6.3f ',i,tgts(i).nunique,tgts(i).pos);
    end
    for j=1:size(like,2)
      fprintf('%7.5f ',like(i,j));
    end
    fprintf('\n');
  end
end
% Assign by highest likelihood
hf=[];
matched=0*[tgts.nunique];
exitted=zeros(1,length(h));
while length(like)>1
  [maxlike,mpos]=max(like(:));
  [mi,mj]=ind2sub(size(like),mpos);
  if maxlike<minimumlike
    if debug
      fprintf('Likelihood of next alternative = %g, skipping\n',maxlike);
    end
    break;
  end
  if mi==ntgts+1
    % Exit
    if debug
      fprintf('Exit by hypo %d\n', h(mj).id);
    end
    like(:,mj)=0;
    exitted(mj)=1;
    continue;   % Don't add hypo
  end
  if mj<=length(h) && (isempty(hf) || ~ismember(h(mj).id,[hf.id]))
    id=h(mj).id;
    entrytime=h(mj).entrytime;
    interval=(snap.when-h(mj).lasttime)*3600*24;
    % Direction vector per second
    delta=(newpos{mi,mj}-h(mj).pos)/interval;
    spd=norm(delta);
    heading=atan2d(delta(2),delta(1));
  elseif mj==length(h)+1
    % New entry
    id=snap.nextid;
    snap.nextid=snap.nextid+1;
    entrytime=snap.when;
    delta=nan;spd=nan;heading=nan;
    if debug
      fprintf('New entry with hypo %d\n', id);
    end
  else
    % More than one target for this hypo
    error('Assert failed\n');
    id=snap.nextid;
    snap.nextid=snap.nextid+1;
    entrytime=snap.when;
    delta=nan;spd=nan;heading=nan;
    fprintf('Multiple targets correspond to prior hypothesis %d - adding hypo %d\n',h(mj).id, id);
  end
  hf=[hf,struct('id',id,'pos',newpos{mi,mj},'tnum',mi,'like',like(mi,mj),'entrytime',entrytime,'lasttime',snap.when,'area',tgts(mi).area,'delta',delta,'speed',spd,'heading',heading,'orientation',tgts(mi).orientation,'minoraxislength',tgts(mi).minoraxislength,'majoraxislength',tgts(mi).majoraxislength)];
  if debug
    fprintf('Assigned target %d (%6.3f,%6.3f) to hypo %d with dist=%.2f, like=%.3f\n', mi, newpos{mi,mj}, id, dist(mi,mj), like(mi,mj));
  end
  matched(mi)=matched(mi)+1;
  % Adjust for this target also matching another hypothesis (convergence of hypotheses)
  pconverge=sum(pmultiple(mi,matched(mi)+1:end))/sum(pmultiple(mi,matched(mi):end));  
  like(mi,:)=like(mi,:)*pconverge;
  like(:,mj)=0;  % Done with this hypo
end
for i=1:length(h)
  if ~exitted(i) && (isempty(hf) || ~ismember(h(i).id,[hf.id]))
    % No target assigned
    lostdur=snap.when-h(i).lasttime;
    if debug
      fprintf('No target assigned to hypothesis %d (lost for %.1f seconds)\n', h(i).id, lostdur*24*3600);
    end
    if lostdur*24*3600>droptime
      if debug
        fprintf('Dropping hypothesis %d after unseen for %.1f seconds\n', h(i).id,lostdur*24*3600);
      end
    else
      hf=[hf,h(i)];
      hf(end).speed=nan;
      hf(end).area=nan;
    end
  end
end
if debug
  missed=find(~matched & [tgts.nunique]>minunique);
  for i=1:length(missed)
    t=tgts(missed(i));
    fprintf('Target %d at (%.3f,%.3f) with %d unique rays, maxlike=%g, not matched\n', missed(i), t.pos, t.nunique,max(like(missed(i),:)));
  end
end

% Adjust areas of hypotheses that match the same target as another
for i=1:ntgts
  if matched(i)>1
    if debug
      fprintf('Scaling area of hypotheses matching target %d by %d\n', i, matched(i));
    end
    for j=1:length(hf)
      if hf(j).tnum==i
        % Rescale area
        hf(j).area=hf(j).area/matched(i);
        % Uncertain about other stats
        hf(j).minoraxislength=nan;
        hf(j).majoraxislength=nan;
        hf(j).orientation=nan;
        % Shift position by half of majoraxis towards prior position so that it will most likely retain correct identity on separation
        % NO LONGER NEEDED - locking onto closest place in region 
        % oldind=find([h.id]==hf(j).id,1);
        % if ~isempty(oldind)
        %   oldpos=h(oldind).pos;
        %   dir=oldpos-hf(j).pos; dir=dir/norm(dir);
        %   hf(j).pos=hf(j).pos+tgts(i).majoraxislength/4*dir;
        % end
      end
    end
  end
end
    
snap.hypo=hf;
snap.like=like;
