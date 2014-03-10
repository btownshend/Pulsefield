% Fit a circle to a set of points
function [center,r,err]=fitcircle(pts,minradius,maxradius)
debug=0;

r0=norm([max(pts(:,1))-min(pts(:,1)),max(pts(:,2))-min(pts(:,2))])/2;
c0=mean(pts,1);
x0=[c0,r0];
if debug
  options=optimset('Display','final');
else
  options=optimset('Display','none');
end
x=fminsearch(@(x) calcerror(x(1:2),x(3),minradius,maxradius,pts),x0,options);
center=x(1:2);
r=x(3);
err=sqrt(calcerror(x(1:2),x(3),minradius,maxradius,pts));
if debug
  setfig('fitcircle');clf;
  plot(pts(:,1),pts(:,2),'x');
  hold on;
  plot(center(1),center(2),'o');
  angle=-pi:.01:pi;
  [x,y]=pol2cart(angle,r);
  x=x+center(1);
  y=y+center(2);
  plot(x,y);
  axis equal
end

function err2=calcerror(center,r,minradius,maxradius,pts)
d=sqrt((pts(:,1)-center(1)).^2+(pts(:,2)-center(2)).^2);
err2=mean((d-r).^2);
if r>maxradius
  err2=err2+100*(r-maxradius)^2;
elseif r<minradius
  err2=err2+100*(r-maxradius)^2;
end
