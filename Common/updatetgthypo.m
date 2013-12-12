% Update current hypothesis of locations 
% Input:
%   h - prior hypotheses
%   snap - snapshot including target structure for current sample
%   samp - sample number (for debug output)
% Output:
%   snap - w/hypo field added, snap.nextid updated
function snap=updatetgthypo(layout,prevsnap,snap,samp)
debug=0;
% PARAMETERS
minunique=5;  % Could be ghost if less than this many unique rays
pghost=0.9;	% with this prob

% Matching
speedmu=0.5;	% Mu of exponential distribution for PDF of speeds expected (m/s)
falselifetime=1;  % Mean lifetime of false signals (seconds)
muhidden=1;     % Mean lifetime of hidden people (not appearing in any target, but still present)
minimumlike=.1;  % Minimum likelihood (prob) to accept a matchup
poserror=0.1;    % Mean position error
maxspeed = 1.3;   % Max speed when extrapolating

% Entries
entryrate=1.0;	% conditional entry rate -- given that something is near the door (entries/second)

% Exits
droptime=5;   % Time to drop a missed target in seconds
exitdroptime=1;  % Time to drop when in range of an exit
exitrate=1.0;  % conditional exit rate -- given that they are beside the door (exits/seond)

% Groups
maxpersons=4;     % Maximum undivided group size
meanpersonarea=0.11;   % In m^2
stdpersonarea=0.04;    % Stddev of person area

tgts=snap.tgts;
ntgts=length(tgts);
h=prevsnap.hypo;  % Start off by continuing old hypotheses
snap.nextid=prevsnap.nextid;
dt=(snap.when-prevsnap.when)*24*3600;

if 0
% Extrapolate position using estimated velocity
for j=1:length(h)
  speed=norm(h(j).velocity);
  if isfinite(speed)
    h(j).pos=h(j).pos+h(j).velocity/speed*min(maxspeed,speed)*dt;
    if ~inpolygon(h(j).pos(1),h(j).pos(2),layout.active(:,1),layout.active(:,2))
      h(j).pos=prevsnap.hypo(j).pos;
      if debug
        fprintf('Extrapolating position from (%f,%f) would end up outside, skipping extrapolation\n',h(j).pos);
      end
    end
  end
end
end

% Calculate probability that each target is multiple people
pmultiple=zeros(ntgts,maxpersons);
for i=1:ntgts
  for j=1:maxpersons
    pmultiple(i,j)=normpdf(tgts(i).area,meanpersonarea*j,stdpersonarea*sqrt(j));
  end
  pmultiple(i,:)=pmultiple(i,:)/sum(pmultiple(i,:));
%  like(i,:)=like(i,:)*dot(pmultiple(i,:),1:maxpersons);
end

% Calculate likelihood matrix of each hypothesis matching each target
dist=nan(ntgts,length(h));
like=dist;
newpos=cell(ntgts,length(h));
for i=1:ntgts
  for j=1:length(h)
    % Find distance to all pixels of target
    pixdist=sqrt((tgts(i).pixellist(:,1)-h(j).pos(:,1)).^2+(tgts(i).pixellist(:,2)-h(j).pos(:,2)).^2);
    % Use closest pixel position as new position
    [dist(i,j),closest]=min(pixdist);
    % But also push towards middle (get 90% of way to center in 3 frames (.5^3=1-0.9)  )
    newpos{i,j}=0.5*tgts(i).pixellist(closest,:)+0.5*tgts(i).pos;
    % Although setting dist to zero if inside range of target would make sense for the probabilities,
    % it doesn't allow selection between multiple possible overlapping targets
    % So, make it monotonically increasing with distance from tgts(i).pos
    dist(i,j)=norm(newpos{i,j}-h(j).pos);

    % Scale down effective distance if we are inside the diameter of the target
    if dist(i,j)<tgts(i).majoraxislength/2
      dist(i,j)=dist(i,j)/2;
    end

    % Weight inversely by distance 
    tlastseen=(snap.when-h(j).lasttime)*24*3600;
    pspeed=exppdf(dist(i,j)/tlastseen,speedmu);
    pdir=poserror/max(poserror,dist(i,j));   % Prob of heading in that direction

    % Decrease likelihood of missing ones linearly with time since last seen
    phidden=exppdf(tlastseen,muhidden);
    % Decrease likelihood for new ones
    existent=(snap.when-h(j).entrytime)*24*3600;
    pfalse=1-expcdf(existent,falselifetime);   % Probability of this being a false signal

    % Estimate of likelihood that target i is the same as hypo j (heuristic)
    like(i,j)=pspeed*pdir*(1-pfalse)*phidden;
    if (debug)
      fprintf('like(%d,%d)=%f dist=%f, pspeed=%f, pdir=%f, 1-pfalse=%f, phidden=%f\n', i,j,like(i,j),dist(i,j),pspeed,pdir,1-pfalse,phidden);
    end
  end
  % Decrease likelhood of targets that don't have unique rays
  if tgts(i).nunique<minunique
    like(i,:)=like(i,:)*(1-pghost);
  end
end

% Calculate entry likelihoods
for i=1:ntgts
  % Add a new possible hypothesis corresponding to an entry
  entrydist(i)=disttoentry(layout,tgts(i).pixellist);
  
  % dist(i,j)=norm(tgts(i).pos-layout.entry);  % Distance from entry
  %  pspeed=exppdf(entrydist(i)/(dt/2),speedmu);   % Prob they moved to here if they just entered
  pspeed=1-expcdf(entrydist(i)/dt,speedmu);   % Prob they moved to here if they just entered
  pentry=0.1; % expcdf(dt,1/entryrate);   % Prob they entered in last dt seconds

  entrylike(i)=pspeed*pentry;
  if (debug)
    fprintf('entry T%d: dist=%f, like=%f\n', i, entrydist(i), entrylike(i));
  end
end

  

% Normalize likelihood of targets with unique rays to a total of 1.0
for i=1:ntgts
  if tgts(i).nunique>=minunique  && sum(like(i,:))+entrylike(i)<1
    % Scale to move total towards 1.0, but not exactly or all tgts in this situation will be interchangeable
    % So, instead take average of current value and 1.0
    sc=0.5+0.5/(sum(like(i,:))+entrylike(i));
    like(i,:)=like(i,:)*sc;
    entrylike(i)=entrylike(i)*sc;
  end
end

% Calculate exit likelihoods
for j=1:length(h)
  % Add a new possible hypothesis corresponding to an exit
  exitdist(j)=disttoentry(layout,h(j).pos);
  interval=(snap.when-h(j).lasttime)*3600*24;  % Time since last seen
  pspeed=exppdf(exitdist(j)/interval,speedmu);   % Prob they moved to here
  pexit=expcdf(interval,1/exitrate);   % Prob they exited in last dt seconds
  exitlike(j)=pspeed*pexit;
end

% Allow people to appear/disappear anywhere with low prob
% Done after normalization
if exist('entrylike')
  entrylike=max(0.01,entrylike);
end
if exist('exitlike')
  exitlike=max(0.01,exitlike);
end

% Dump values
if debug && length(like)>0
  fprintf('\n%3d Uniq  XPos   YPos  ',samp);
  for j=1:size(like,2)
    fprintf('  H%-3d  ',h(j).id);
  end
  fprintf(' Enter\n');
  for i=1:size(like,1)
    if all(isnan(like(i,:)))
      continue;
    end
    fprintf('T%-2d %3d %6.3f %6.3f ',i,tgts(i).nunique,tgts(i).pos);
    for j=1:size(like,2)
      fprintf('%7.5f ',like(i,j));
    end
    fprintf('%7.5f\n',entrylike(i));
  end
  if length(h)>0
    fprintf('Exit                  ');
    fprintf('%7.5f ',exitlike);
    fprintf('\n');
  end
end
origlike=like;
if exist('entrylike')
  origlike(1:length(entrylike),end+1)=entrylike;
end
if exist('exitlike')
  origlike(end+1,1:length(exitlike))=exitlike;
end

if samp==-1
  keyboard
end

% Assign by highest likelihood
hf=struct('id',{},'pos',{},'tnum',{},'like',{},'entrytime',{},'lasttime',{},'area',{},'velocity',{},'heading',{},'orientation',{},'minoraxislength',{},'majoraxislength',{},'groupid',{},'groupsize',{});

matched=0*[tgts.nunique];
while ~isempty(like)
  [maxlike,mpos]=max(like(:));
  [mi,mj]=ind2sub(size(like),mpos);
  if maxlike<minimumlike
    if h(mj).groupsize>1 && maxlike>minimumlike/5
      if debug
        fprintf(['Likelihood of next alternative = %g, but it is in ' ...
                 'a group so not skipping\n'], maxlike);
      end
    else
      if debug
        fprintf('Likelihood of next alternative = %g, skipping\n',maxlike);
      end
      break;
    end
  end
  if isempty(hf) || ~ismember(h(mj).id,[hf.id])
    id=h(mj).id;
    entrytime=h(mj).entrytime;
    interval=(snap.when-h(mj).lasttime)*3600*24;
    % Direction vector per second
    % h(mj).pos already has a velocity prediction factor in it; go back to previous point estimate
    velocity=(newpos{mi,mj}-prevsnap.hypo(mj).pos)/interval;
    heading=atan2d(velocity(2),velocity(1));
  end
  hf=[hf,struct('id',id,'pos',newpos{mi,mj},'tnum',mi,'like',like(mi,mj),'entrytime',entrytime,'lasttime',snap.when,'area',tgts(mi).area,'velocity',velocity,'heading',heading,'orientation',tgts(mi).orientation,'minoraxislength',tgts(mi).minoraxislength,'majoraxislength',tgts(mi).majoraxislength,'groupid',id,'groupsize',1)];
  if debug
    fprintf('Assigned target %d (%6.3f,%6.3f) to hypo %d with dist=%.2f, like=%.3f\n', mi, newpos{mi,mj}, id, dist(mi,mj), like(mi,mj));
  end
  matched(mi)=matched(mi)+1;
  % Adjust for this target also matching another hypothesis (convergence of hypotheses)
  pconverge=sum(pmultiple(mi,matched(mi)+1:end))/sum(pmultiple(mi,matched(mi):end));  
  like(mi,:)=like(mi,:)*pconverge;
  entrylike(mi)=0;   % unlikely that it just picked up an entry as well
  like(:,mj)=0;  % Done with this hypo
end

% Check for exits or disappearances
for i=1:length(h)
  if isempty(hf) || ~ismember(h(i).id,[hf.id])
    % No target assigned
    lostdur=snap.when-h(i).lasttime;
    if debug
      fprintf('No target assigned to hypothesis %d (lost for %.1f seconds, exitlike=%f)\n', h(i).id, lostdur*24*3600,exitlike(i));
    end

    if exitlike(i)>minimumlike & lostdur*24*3600>exitdroptime  || lostdur*24*3600>droptime
      if debug
        fprintf('Dropping hypothesis %d after unseen for %.1f seconds\n', h(i).id,lostdur*24*3600);
      end
    else
      hf=[hf,h(i)];
      hf(end).area=nan;
      hf(end).tnum=nan;
      hf(end).velocity=[nan nan];
      hf(end).heading=nan;
      hf(end).minoraxislength=nan;
      hf(end).majoraxislength=nan;
    end
  end
end

% Missed targets -- could be entries
% Check what the total  likelihood for unmatched possible entries was in the last frame
unmatched=~ismember(1:length(prevsnap.tgts),[prevsnap.hypo.tnum]);
if sum(unmatched)>0
  unmatchedlike=sum(prevsnap.like(unmatched,end));
  if debug
    fprintf('Carrying forward %d unmatched targets from prior frame with total entry likelihood of %f\n', sum(unmatched), unmatchedlike);
  end
else
  unmatchedlike=0;
end

missed=find(~matched & [tgts.nunique]>minunique);
for i=1:length(missed)
  mi=missed(i);
  t=tgts(mi);
  if debug
    fprintf('Target %d at (%.3f,%.3f) with %d unique rays, maxlike=%g, entrylike=%g+%g, not matched\n', mi, t.pos, t.nunique,max(like(mi,:)),entrylike(mi),unmatchedlike);
  end
  % Check if it could be an entry
  if entrylike(mi)+unmatchedlike>minimumlike
    if 1
      % Check if anyone else is already nearby (to prevent double counting entries)
      nearby=false;
      fprintf('Distance to other hypos is ');
      for j=1:length(hf)
        edist=norm(hf(j).pos-t.pos);
        fprintf('%.2f ', edist);
        nearby=nearby||(edist<0.4);   % TODO - choose value
      end
      if nearby
        fprintf('.  Has close neighbors, not considering as an entry\n');
        continue;
      else
        fprintf('\n');
      end
    end
    % New entry
    id=snap.nextid;
    snap.nextid=snap.nextid+1;
    entrytime=snap.when;
    velocity=[nan,nan];heading=nan;
    hf=[hf,struct('id',id,'pos',t.pos,'tnum',mi,'like',entrylike(mi),'entrytime',entrytime,'lasttime',snap.when,'area',tgts(mi).area,'velocity',velocity,'heading',heading,'orientation',tgts(mi).orientation,'minoraxislength',tgts(mi).minoraxislength,'majoraxislength',tgts(mi).majoraxislength,'groupid',id,'groupsize',1)];
    if debug
      fprintf('Assigned target %d (%6.3f,%6.3f) to NEW hypo %d with dist=%.2f, like=%.3f\n', mi, t.pos, id, entrydist(mi), entrylike(mi));
    end
  else
    % Increment unmatched likelihood by prior unmatched ones
    origlike(mi,end)=origlike(mi,end)+unmatchedlike;
  end
end

% Adjust areas of hypotheses that match the same target as another so they share the area
% Adjust positions to spread them evenly over the target in the same relative order they were previously
for i=1:ntgts
  if matched(i)>1
    if debug
      fprintf('Scaling area of hypotheses matching target %d by %d\n', i, matched(i));
    end
    minx=min(tgts(i).pixellist(:,1));
    maxx=max(tgts(i).pixellist(:,1));
    miny=min(tgts(i).pixellist(:,2));
    maxy=max(tgts(i).pixellist(:,2));
    ingroup=find([hf.tnum]==i);
    xpos=arrayfun(@(z) z.pos(1), hf(ingroup));
    [~,xorder]=sort(xpos);
    ypos=arrayfun(@(z) z.pos(2), hf(ingroup));
    [~,yorder]=sort(ypos);
    if debug
      fprintf('T%d: xpos=%s, xorder=%s, ypos=%s, yorder=%s\n',i,sprintf('%f ',xpos),sprintf('%d ',xorder),sprintf('%f ',ypos),sprintf('%d ',yorder));
    end
    for ij=1:length(ingroup)
      j=ingroup(ij);
      % Rescale area
      hf(j).area=hf(j).area/matched(i);
      % Uncertain about other stats
      hf(j).minoraxislength=nan;
      hf(j).majoraxislength=nan;
      hf(j).orientation=nan;
      % Spread along target, averaging in current position
      prevhypo=find([prevsnap.hypo.id]==hf(j).id);
      if ~isempty(prevhypo)
        prevpos=prevsnap.hypo(prevhypo).pos;
      else
        prevpos=hf(j).pos;
      end
      hf(j).pos(1)=(xorder(ij)/(matched(i)+1)*(maxx-minx)+minx)*0.25+prevpos(1)*0.75;
      hf(j).pos(2)=(yorder(ij)/(matched(i)+1)*(maxy-miny)+miny)*0.25+prevpos(2)*0.75;
      if debug
        fprintf('Adjusted position in group from (%f,%f) to (%f,%f)\n', prevpos,hf(j).pos);
      end
    end
  end
end
    
snap.hypo=hf;
snap.like=origlike;
