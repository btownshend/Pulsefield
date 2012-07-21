% levcheck - check signal levels as a function of amount of drive
function levcheck(p,colors,doplot)
if nargin<2
  doplot=1;
end
clipthresh=250;
levs=[0,1,2,4,8,16,20,24,28,32,36,40,48,64,80,96,127];
%col={[1 1 1], [1 0 0], [0 1 0], [0 0 1], [1 1 0], [1 0 1], [0 1 1]};
col={[1 1 1], [1 0 0], [0 1 0], [0 0 1]};
if doplot
  setfig('levcheck');
  clf;
  pcol='krgbymc';
end

satlev={};maxout={};
for c=1:length(colors)  % Loop over colors (4=white)
  col=colors{c}/max(colors{c});
  outlevs=[];
  clipped=[];
  for l=1:length(levs)
    vis=getvisible(p,1,round(levs(l)*col));
    outlevs(l)=min(nanmedian(vis.lev,2));
    clipped(l)=sum(vis.lev(isfinite(vis.lev(:)))>=clipthresh)/sum(isfinite(vis.lev(:)));
  end
  if doplot
    subplot(2,1,1);
    plot(levs,outlevs,pcol(c));
    hold on;
    xlabel('Drive level');
    ylabel('Signal Level');
    title('Median levels');
    subplot(2,1,2);
    plot(levs,clipped,pcol(c));
    hold on;
    xlabel('Drive level');
    ylabel('Fraction of valid LEDs Clipped');
    title('Clipping Count');
  end
  satlev{c}=levs(min(find(outlevs==255)));
  maxout{c}=max(outlevs);
  if isempty(satlev{c})
    colors{c}=col*127;
    fprintf('Warning: Unable to saturate camera with [%d,%d,%d]\n', colors{c});
  else
    colors{c}=col*satlev{c};
  end
end

fprintf('Color drive levels:\n');
for c=1:length(colors)
  fprintf('[%d,%d,%d] -> %d\n', colors{c}, maxout{c});
end
