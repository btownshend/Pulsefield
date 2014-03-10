% Calculate distance from a line define by two points and another point
function d=line2pt(l1,l2,p)
D=l2-l1;
d=abs(D(2)*p(1)-D(1)*p(2)+l1(1)*l2(2)-l2(1)*l1(2))/norm(D);
