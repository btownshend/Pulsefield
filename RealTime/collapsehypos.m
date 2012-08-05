% Collapse snap shots into hypos organized by ID
function h=collapsehypos(recvis);
snap=recvis.snap;
h={};
for i=1:length(snap)
  sh=snap(i).hypo;
  if ~isempty(sh)
    for jj=1:length(sh)
      j=sh(jj).id;
      % Only include if this was a true sighting
      if sh(jj).lasttime == snap(i).when
        if j>length(h) || isempty(h{j})
          h{j}=sh(jj);
        else
          h{j}(end+1)=sh(jj);
        end
      end
    end
  end
end
