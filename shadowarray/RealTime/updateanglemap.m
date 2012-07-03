% plot distortion based on layout and pixcalib
function [p,layout]=updateanglemap(p,layout)
for i=1:length(p.camera)
  c=p.camera(i);
  for j=1:length(c.pixcalib)
    c2ldir=layout.lpos(j,:)-layout.cpos(i,:);
    ad(j)=cart2pol(layout.cdir(i,1),layout.cdir(i,2))-cart2pol(c2ldir(1),c2ldir(2));
    pix(j)=c.pixcalib(j).pos(1);
  end
  sel=isfinite(pix);
  fit=polyfit(2*pix(sel)/c.hpixels-0.5,ad(sel),1)
  p.camera(i).anglemap=polyval(fit,2*(0:c.hpixels-1)/c.hpixels-0.5);
  % Recenter
  addangle=p.camera(i).anglemap(round((end+1)/2))
  [layout.cdir(i,1),layout.cdir(i,2)]=pol2cart(cart2pol(layout.cdir(i,1),layout.cdir(i,2))-addangle,1);
  p.camera(i).anglemap=p.camera(i).anglemap-addangle;
end
