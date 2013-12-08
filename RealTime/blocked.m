% Summarize which LEDs are blocked in a vis structure
function blocked(p,vis)
labels={'In use','Blocked','Visible','Indeterminate','High Var'};
cnts=zeros(1,length(labels));
fprintf('          ');
for i=1:length(labels)
  fprintf('%10s ',labels{i});
end
fprintf('\n');
for c=1:size(vis.v,1)
  inuse=p.camera(c).viscache.inuse;
  fprintf('Camera %d: ', c);
  for i=1:length(labels)
    if i==1
      n=sum(inuse);
    else
      n=sum((vis.vorig(c,:)==i-2)&inuse);
    end
    fprintf('%10.0f ',n);
    cnts(i)=cnts(i)+n;
  end
  blocked=find(vis.v(c,:)==0);
  if length(blocked)>0
    fprintf('       Blocked: %s',shortlist(blocked));
  end
  fprintf('\n');
end


fprintf('%-9s ','Total');
for i=1:length(cnts)
  fprintf('%10.0f ',cnts(i));
end
fprintf('\n');
