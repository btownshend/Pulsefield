% findgroups - find groups in a set of hypo
function [info,snap]=findgroups(info,snap,prevsnap)
for i=1:length(snap.hypo)
  if isnan(snap.hypo(i).tnum)
    ph=find([prevsnap.hypo.id]==snap.hypo(i).id,1);
    if isempty(ph)
      fprintf('Hypo %d has tnum=nan, no corresponding entry in prevsnap, assuming not in group\n', i);
    else
      snap.hypo(i).groupid=prevsnap.hypo(ph).groupid;
      snap.hypo(i).groupsize=prevsnap.hypo(ph).groupsize;
      fprintf('Hypo %d has no attributed target, copying groupid %d, groupsize %d from prevsnap\n', snap.hypo(i).id, snap.hypo(i).groupid, snap.hypo(i).groupsize);
    end
  else
    % Compute groupid -- lowest id in group, or 0 if not in a group
    sametnum=find(snap.hypo(i).tnum==[snap.hypo.tnum]);
    if length(sametnum)<=1  % Could be one if the hypo doesn't have a matching tgt (such as when exitting)
      snap.hypo(i).groupid=0;
      snap.hypo(i).groupsize=1;
    else
      snap.hypo(i).groupid=info.groupmap.idset2gid([snap.hypo(sametnum).id]);
      snap.hypo(i).groupsize=length(sametnum);
    end
  end
end
