function undistortplot(distortion,flen,swidth,sheight,col)
if nargin<5
  col='b';
end
r=0:.01:4;
r2=r.^2;
rdistort=1+distortion(1)*r2+distortion(2)*r2.^2+distortion(5)*r2.^3;
rdistort=rdistort ./ (1+distortion(6)*r2+distortion(7)*r2.^2+distortion(8)*r2.^3);
pix=r.*rdistort*flen;
sel=pix<=norm([swidth,sheight])/2;
plot(r(sel),pix(sel),col);
xlabel('X/Z');
ylabel('Pixel Radius');
c=axis;
hold on;
plot([c(1),c(2)],[swidth,swidth]/2,':');
plot([c(1),c(2)],[sheight,sheight]/2,':');
maxradius=norm([swidth,sheight])/2;
plot([c(1),c(2)],maxradius*[1,1],':');
maxratio=max(r(pix>=0&pix<=maxradius));
axis([0,maxratio,0,maxradius*1.1]);
return;

mapx=zeros(12,16);
mapy=mapx;
x=mapx;
y=mapx;
sf=max(size(mapx))-1;
for i=1:size(x,2)
  x(:,i)=(0:size(x,1)-1)*2/sf-(size(x,1)-1)/sf;
end
for j=1:size(y,1)
  y(j,:)=(0:size(y,2)-1)*2/sf-(size(y,2)-1)/sf;
end
x=x*3; y=y*3;
r2=x.^2+y.^2;
rdistort=1+distortion(1)*r2+distortion(2)*r2.^2+distortion(5)*r2.^3;
rdistort=rdistort ./ (1+distortion(6)*r2+distortion(7)*r2.^2+distortion(8)*r2.^3);
mapx=(x.*rdistort);
mapy=(y.*rdistort);
sel=(abs(mapx)<1) & (abs(mapy)<1);

setfig('distortion'); clf;
subplot(211);
plot(sqrt(r2(sel)),r2(sel).*rdistort(sel),'.');
xlabel('Radius');
ylabel('Distortion');
subplot(212);
quiver(x(sel),y(sel),mapx(sel)-x(sel),mapy(sel)-y(sel))
axis equal