% Plot variance of pixels
function varplot(p,vis)
setfig('varplot');
ncam=length(p.camera);
for c=1:ncam
  var=vis.refim2{c};
  subplot(ncam/2,2,c);
  cdfplot(sqrt(var(:))*255);
  %    set(gca,'XScale','log');
    set(gca,'YScale','log');
  axis([0,10,0.01,1]);
  s=sort(var(:),'ascend');
  pct1=s(round(length(s)/100));
  title(sprintf('Camera %d - 1-percentile std=%.2f',c,sqrt(pct1)*255));
end
suptitle('Distribution of SDev in pixel values');