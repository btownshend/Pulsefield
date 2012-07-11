% Find intersection of two lines
% Lines are each defined by two points (in 2D)  p1<->p2 and p3<->p4
function p=lineintersect(p1,p2,p3,p4)
x1=p1(1);y1=p1(2);
x2=p2(1);y2=p2(2);
x3=p3(1);y3=p3(2);
x4=p4(1);y4=p4(2);
den=(x1-x2)*(y3-y4)-(y1-y2)*(x3-x4);
p(1)=((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4))/den;
p(2)=((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4))/den;
