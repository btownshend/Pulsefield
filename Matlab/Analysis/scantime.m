function t=scantime(pt)
% Compute timing for lidar
f=50;  % Scan rate/s
res=1/3;   % Resolution in degrees

% Compute timing for a particular x,y point
p1=[-20,0];
p2=[20,0];
rel1=pt-p1;
rel2=pt-p2;
a1=cart2pol(rel1(1),rel1(2));
a2=cart2pol(rel2(1),rel2(2));
t1=a1/(2*pi)/f;
t2=a2/(2*pi)/f;
t=[t1,t2];
