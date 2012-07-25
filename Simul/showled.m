% Show rays from camera to LED based on calibration
function showled(p,led)
layout=p.layout;
plotlayout(layout)
hold on;
lpos=layout.lpos(led,:);
for i=1:length(p.camera)
  c=p.camera(i);
  spos=c.pixcalib(led).pos(1);
  if isfinite(spos)
    sa=spos2angle(spos,c);
    s=layout.cpos(i,:);
    len=norm(s-lpos);
    [d(1),d(2)]=pol2cart(cart2pol(layout.cdir(i,1),layout.cdir(i,2))-sa,len);
    e=s+d;
    plot([s(1),e(1)],[s(2),e(2)]);
  end
end
title(sprintf('Projected position of LED %d',led));
axis([lpos(1)-0.2,lpos(1)+0.2,lpos(2)-0.2,lpos(2)+0.2]);