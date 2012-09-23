% Test load from Ableton
al1=Ableton;
al1.update
al2=Ableton;
al2.update
nerr=0;
for i=1:size(al1.clipnames,1)
  for j=1:size(al1.clipnames,2)
    if ~strcmp(al1.clipnames{i,j},al2.clipnames{i,j}) && ~(isempty(al1.clipnames{i,j}) && isempty(al2.clipnames{i,j}))
      fprintf('al1(%d,%d)=%s, al2=%s\n', i,j,al1.clipnames{i,j}, al2.clipnames{i,j});
      nerr=nerr+1;
    end
  end
end
fprintf('Total of %d mismatches\n', nerr);