% Plot LIDAR coverage at 3:00 plaza
plazadiam=250*12/39.37;   % In meters
sep=40*12/39.37;    % LIDAR separation
scan=190;    % Scan in degrees
[p(1,1),p(1,2)]=pol2cart(45*pi/180,sep/2);
[p(2,1),p(2,2)]=pol2cart((45+180)*pi/180,sep/2);
setfig('plaza');clf;
plot(p(:,1),p(:,2),'x');
hold on;
axis equal;
th=0:.01:2*pi;
edge=nan(length(th),2);
[edge(:,1),edge(:,2)]=pol2cart(th,plazadiam/2);
plot(edge(:,1),edge(:,2));
range=30;
a=-95:95
a1=a-135;
d=nan(length(a1),2);
[d(:,1),d(:,2)]=pol2cart(a1*pi/180,range);
d=[0 0;d;0 0];
d(:,1)=d(:,1)+p(1,1);
d(:,2)=d(:,2)+p(1,2);
plot(d(:,1),d(:,2),'-');
plot(-d(:,1),-d(:,2),'-');

