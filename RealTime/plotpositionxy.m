% Plot position as a function of sample count in x and y direction separately
% Usage: plotpositionxy(tgtests)
%  tgtests - cell array of tgtestimates as returned by analyze
function plotpositionxy(tgtests)
setfig('plotpositionxy');
clf
hold on;
for i=1:length(tgtests)
  for j=1:size(tgtests{i}.tpos,1)
    plot(i,tgtests{i}.tpos(j,1),'.r');
    plot(i,tgtests{i}.tpos(j,2),'.g');
  end
end
xlabel('Sample');
ylabel('Position (m)');

