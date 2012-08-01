setfig('track');
clf;
hold on;
col='rgbmck';

while true
  disp('Measuring');
  [v,lev]=getvisible(p,doplot>1);

  % Analyze data to estimate position of targets using layout
  disp('Analyzing');
  [possible,tgtestimate]=analyze(p,v,doplot);
  
  setfig('track');
  for i=1:size(tgtestimate.tpos,1)
    fprintf('%f,%f\n',tgtestimate.tpos(i,:));
    plot(tgtestimate.tpos(i,1),tgtestimate.tpos(i,2),['o',col(i)]);
  end
end

