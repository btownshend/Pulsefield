% findgroups - find groups in a set of hypo
function [info,snap]=findgroups(info,snap,prevsnap)
for i=1:length(snap.hypo)
  if isnan(snap.hypo(i).tnum)
    fprintf('Hypo %d has tnum=nan, assuming not in group\n', i);
    snap.hypo(i).groupid=0;
    snap.hypo(i).groupsize=1;
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
