% levcheck - check signal levels as a function of amount of drive
function colors=levcheck(p,doplot)
if nargin<2
  doplot=1;
end
levs=[0,1,2,4,8,16,20,24,28,32,36,40,48,64,80,96,127];
col={[1 1 1], [1 0 0], [0 1 0], [0 0 1], [1 1 0], [1 0 1], [0 1 1]};
if doplot
  setfig('levcheck');
  clf;
  hold on;
  pcol='krgbymc';
end

satlev={};maxout={};
for c=1:length(col)  % Loop over colors (4=white)
  outlevs=[];
  for l=1:length(levs)
    vis=getvisible(p,1,levs(l)*col{c});
    outlevs(l)=nanmedian(vis.lev(:));
  end
  if doplot
    plot(levs,outlevs,pcol(c));
  end
  satlev{c}=levs(min(find(outlevs==255)));
  maxout{c}=max(outlevs);
  if isempty(satlev{c})
    colors{c}=col{c}*127;
    fprintf('Warning: Unable to saturate camera with [%d,%d,%d]\n', colors{c});
  else
    colors{c}=col{c}*satlev{c};
  end
end
if doplot
  xlabel('Drive level');
  ylabel('Signal Level');
  title('Median levels');
end

fprintf('Color drive levels:\n');
for c=1:length(colors)
  fprintf('[%d,%d,%d] -> %d\n', colors{c}, maxout{c});
end
