% plot distortion based on layout and pixcalib
function anglemap=plotdistortion(p)
layout=p.layout;
setfig('plotdistortion');
clf;
hold on;
lpos=layout.lpos;
for i=1:length(p.camera)
  c=p.camera(i);
  ctr=[c.hpixels,c.vpixels]/2;
  cdir=layout.cdir(i,:);
  cdir=cdir/norm(cdir);
  cpos=layout.cpos(i,:);
  for j=2:length(c.pixcalib)
    pc=c.pixcalib(j);
    dpixel(i,j)=norm(c.pixcalib(j).pos-c.pixcalib(j-1).pos);
    c2ldir=lpos(j,:)-cpos;
    c2ldir=c2ldir/norm(c2ldir);
    c2ldir2=lpos(j-1,:)-cpos;
    c2ldir2=c2ldir2/norm(c2ldir2);
    c2ldiravg=(c2ldir+c2ldir2);
    c2ldiravg=c2ldiravg/norm(c2ldiravg);
    ad(i,j)=cart2pol(cdir(1),cdir(2))-cart2pol(c2ldiravg(1),c2ldiravg(2));
    %    ad(i,j)=acos((dot(c2ldiravg,cdir)));
    dangle(i,j)=cart2pol(c2ldir2(1),c2ldir2(2))-cart2pol(c2ldir(1),c2ldir(2));
    %    dangle(i,j)=acos((dot(c2ldir,c2ldir2)));
  end
end
dangle=real(dangle);  % Some small fuzz sometimes causes acos(>1) -> imaginary part
subplot(211);
plot(ad'*180/pi,(dpixel./(dangle * 180/pi))','.');
hold on;
% Plot theoretical
cmap=get(gcf,'Colormap');
for i=1:length(p.camera)
  cam=p.camera(i);
  plot((cam.anglemap(1:end-1)+cam.anglemap(2:end))/2*180/pi,1 ./(diff(cam.anglemap)*180/pi),'Color',cmap(i,:));
end
xlabel('Angle to right of center');
ylabel('Pixels/Degree');
c=axis;
axis([-90,90,max([0,c(3)]),min([50,c(4)])]);
title('Delta pixels/degree');

% Plot cumulative
subplot(212);
dpda=dpixel./dangle;
dpda=dpda(:);
sel=isfinite(dpda) & abs(dpda)<10000;
dpda=dpda(sel);
[stheta,ord]=sort(ad(sel));
dpda=dpda(ord);
dtheta=[0;diff(stheta)];
px=cumsum(dpda.*dtheta);
%plot(stheta*180/pi,cumsum(dpda)./(1:length(dpda))'.*stheta,'-');
plot(stheta*180/pi,px,'-');
hold on;
%initslope=mean(dpda(1:floor(end/4)));
%plot([0,stheta(end)]*180/pi,[0,stheta(end)*initslope],':r');
am=p.camera(1).anglemap;
plot(am*180/pi,(1:length(am))-1,'r:');
xlabel('Angle from center');
ylabel('Pixel Coordinate');
axis([-90,90,0,cam.hpixels]);
title('Absolute pixels vs angle');

suptitle('Distortion');

[upx,ord]=unique(px,'stable');
anglemap=interp1(px(ord),stheta(ord),0:cam.hpixels-1,'pchip');
