% Plot the assignment map cost
function plotassignmentcosts(snap)
minc=nan(1,length(snap));
minc2=nan(1,length(snap));
for i=1:length(snap)
  c=snap(i).tracker.cost;
  if isempty(c)
    continue;
  end
  [minc(i),mpos]=min(c(:));
  [j,k]=ind2sub(size(c),mpos);
  c=c([1:j-1,j+1:end],[1:k-1,k+1:end]);
  if isempty(c)
    continue;
  end
  [minc2(i),mpos]=min(c(:));
end

setfig('plotassignementcosts');clf;
plot(minc,'g');
hold on;
plot(minc2,'r');
xlabel('Index');
ylabel('Minimum Cost');
c=axis;
c(4)=max(minc)*2;
c(3)=floor(min(minc));
c(1)=find(isfinite(minc),1);
c(2)=find(isfinite(minc),1,'last');
axis(c);