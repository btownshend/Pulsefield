function d=linecircle(p1,p2,c,r)
debug=1;
x1=p1(1)-c(1);y1=p1(2)-c(2);
x2=p2(1)-c(1);y2=p2(2)-c(2);
D=x1*y2-x2*y1;
dx=x2-x1;
dy=y2-y1;
dr=norm([dx,dy]);
rad=(r*dr)^2-D^2;
if rad<0
  d=inf;
else
  x=(D*dy+sign(dy)*dx*sqrt(rad))/(dr^2);
  y=(-D*dx+abs(dy)*sqrt(rad))/(dr^2);
  d1=norm([x1-x,y1-y]);
  if debug
    fprintf('(%.2f,%.2f)->(%.2f,%.2f) intersects (%.2f,%.2f),r=%.2f at (%.2f,%.2f),d=%.2f',p1,p2,c,r,x+c(1),y+c(2),d1);
  end
  x=(D*dy-sign(dy)*dx*sqrt(rad))/(dr^2);
  y=(-D*dx-abs(dy)*sqrt(rad))/(dr^2);
  d2=norm([x1-x,y1-y]);
  if debug
    fprintf('and (%.2f,%.2f),d=%.2f\n',x+c(1),y+c(2),d2);
  end
  d=min(d1,d2);
end
