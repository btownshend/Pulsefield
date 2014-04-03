% Calculate distance from a line segment define by two points and another point
% L2 may be a matrix
function d=segment2pt(l1,l2,p)
D(:,1)=l2(:,1)-l1(:,1);
D(:,2)=l2(:,2)-l1(:,1);
D2=D(:,1).^2+D(:,2).^2;
p1(:,1)=p(:,1)-l1(:,1);
p1(:,2)=p(:,2)-l1(:,2);
% u is projection of p onto extended line to give length from origin of closest point
u=(p1(:,1).*D(:,1)+p1(:,2).*D(:,2))./D2;
% Bound in segment
u=min(1,max(0,u));
% Get x,y position 
t(:,1)=l1(:,1)+u.*D(:,1);
t(:,2)=l1(:,2)+u.*D(:,2);
% distance
d=sqrt((p(:,1)-t(:,1)).^2+(p(:,2)-t(:,2)).^2);
