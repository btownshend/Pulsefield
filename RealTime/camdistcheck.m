% Check pixcalib structure against camera distortion measurements
function camdistcheck(p)
cam=2;
pc=p.camera(cam).pixcalib;

for i=1:length(pc)
  pos(i,:)=pc(i).pos;
  mappos(i,:)=undistort(pos(i,:));
  angle(i)=atan(norm(mappos(i,:)));
  if (mappos(i,1)<0)
    angle(i)=-angle(i);
  end
  vec=p.layout.lpos(i,:)-p.layout.cpos(cam,:);
  langle(i)=cart2pol(p.layout.cdir(cam,1),p.layout.cdir(cam,2))-cart2pol(vec(1),vec(2));
end
angle=unwrap(angle)*180/pi;
langle=unwrap(langle)*180/pi;
if nanmean(angle)>90
  angle=angle-180;   % Flipped over
end

setfig('camdistcheck.1');clf;
plot(pos(:,1),pos(:,2),'.');hold on;
plot(pos(1,1),pos(1,2),'.g');
plot(pos(end,1),pos(end,2),'.r');

setfig('camdistcheck.2');clf;
plot(mappos(:,1),mappos(:,2),'.');hold on;
plot(mappos(1,1),mappos(1,2),'.g');
plot(mappos(end,1),mappos(end,2),'.r');

setfig('camdistcheck.3');clf;
offset=nanmean(langle-angle)
aerror=nanstd(langle-angle);
subplot(211);
plot(angle);
hold on;
plot(langle-offset,'g');
xlabel('LED');
ylabel('Angle (degrees)');
subplot(212);
plot(mod(angle-langle+offset+180,360)-180,'r');
xlabel('LED');
ylabel('Position error (degrees)');
title(sprintf('Computed/Expected Angle (Std=%.2f degrees)',aerror));


function x=undistort(xd)
fc = [ 1602.62137   1602.23155 ]/2;
cc = [ 2053.08166   1259.10321 ]/2;
kc = [ -0.06183   0.00166   -0.00124   -0.00023 ];
xd=(xd-cc)./fc;
x=comp_fisheye_distortion(xd',kc)';


