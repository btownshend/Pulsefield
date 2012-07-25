% Update angle map using observed LED->pixel mapping
function p=updateanglemap(p)
usepoly=0;
for i=1:length(p.camera)
  c=p.camera(i);
  for j=1:length(c.pixcalib)
    c2ldir=p.layout.lpos(j,:)-p.layout.cpos(i,:);
    ad(j)=cart2pol(p.layout.cdir(i,1),p.layout.cdir(i,2))-cart2pol(c2ldir(1),c2ldir(2));
    pix(j)=c.pixcalib(j).pos(1);
  end
  sel=isfinite(pix)&(pix>350)&(pix<c.hpixels-350);
  if usepoly
    % Fit points to a polynomial
    fit=polyfit(2*pix(sel)/c.hpixels-0.5,ad(sel),6);
    p.camera(i).anglemap=polyval(fit,2*(0:c.hpixels-1)/c.hpixels-0.5);
  else
    pix=pix(sel);
    ad=ad(sel);
    [pix,ia]=unique(pix);
    ad=ad(ia);
    [pix,ord]=sort(pix);
    %ad=ad(ord);  % This can make an out-of-order seq
    ad=sort(ad);
    p.camera(i).anglemap=interp1(pix,ad,0:c.hpixels-1,'linear','extrap');
    for j=2:length(p.camera(i).anglemap)
      if p.camera(i).anglemap(j)<=p.camera(i).anglemap(j-1)
        p.camera(i).anglemap(j)=p.camera(i).anglemap(j-1)+eps;
      end
    end
    b=hanning(50);b=b/sum(b);
    p.camera(i).anglemap=filtfilt(b,1,p.camera(i).anglemap);
  end
  % Recenter
  addangle=p.camera(i).anglemap(round((end+1)/2));
  [p.layout.cdir(i,1),p.layout.cdir(i,2)]=pol2cart(cart2pol(p.layout.cdir(i,1),p.layout.cdir(i,2))-addangle,1);
  p.camera(i).anglemap=p.camera(i).anglemap-addangle;
end
