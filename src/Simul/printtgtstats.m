function printtgtstats(p,actual,estimate,maxerror)
% Compute distances between all actual and estimates
for i=1:size(actual,1)
  dist(i,:)=sqrt((actual(i,1)-estimate(:,1)).^2+(actual(i,2)-estimate(:,2)).^2);
end
cnt=zeros(1,size(estimate,1));
fprintf('     Actual           Estimate\n');
for a=1:size(actual,1)
  fprintf('%2d: %6.1f %6.1f ', a, actual(a,:)*100);
  [mindist,e]=min(dist(a,:));
  if mindist<maxerror
    fprintf('-> %6.1f %6.1f d=%6.1f cm', estimate(e,:)*100, mindist*100);
    cnt(e)=cnt(e)+1;
    if cnt(e)>1
      fprintf(' Dupe(%dx)',cnt(e));
    end
    fprintf('\n');
  else
    fprintf('    Not found    d=%6.1f cm\n',mindist*100);
  end
end
% Print remaining ones
remain=find(cnt==0);
for i=1:length(remain)
  fprintf('E%-2d                  %6.1f %6.1f\n', remain(i), estimate(remain(i),:)*100);
end
